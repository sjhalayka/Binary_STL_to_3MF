#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <cstring>
#include <ios>
#include <cmath>
#include <set>
#include <sstream>
#include <iomanip>
#include <string>
using std::string;
using std::setprecision;
using std::ostringstream;
using std::set;
using std::sqrt;
using std::memcpy;
using std::vector;
using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;
using std::ios_base;



class vertex_3
{
public:
	float x;
	float y;
	float z;

	size_t index;


	inline const vertex_3& operator+(const vertex_3 &right) const
	{
		static vertex_3 temp;

		temp.x = this->x + right.x;
		temp.y = this->y + right.y;
		temp.z = this->z + right.z;

		return temp;
	}



	inline bool operator==(const vertex_3 &right) const
	{
		if (right.x == x && right.y == y && right.z == z)
			return true;
		else
			return false;
	}

	inline bool operator<(const vertex_3 &right) const
	{
		if(x < right.x)
			return true;
		else if(x > right.x)
			return false;

		if(y < right.y)
			return true;
		else if(y > right.y)
			return false;

		if(z < right.z)
			return true;
		else if(z > right.z)
			return false;

		return false;
	}

	inline const vertex_3& operator-(const vertex_3 &right) const
	{
		static vertex_3 temp;

		temp.x = this->x - right.x;
		temp.y = this->y - right.y;
		temp.z = this->z - right.z;

		return temp;
	}

	inline const float dot(vertex_3 &rhs) const
	{
		return x*rhs.x + y*rhs.y + z*rhs.z;
	}

	inline const float self_dot(void) const
	{
		return x*x + y*y + z*z;
	}

	inline const float length(void) const
	{
		return sqrt(self_dot());
	}


	inline const void normalize(void)
	{
		float len = length();

		if (0.0f != len)
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}

	inline const vertex_3& cross(const vertex_3 &right) const
	{
		static vertex_3 temp;

		temp.x = y*right.z - z*right.y;
		temp.y = z*right.x - x*right.z;
		temp.z = x*right.y - y*right.x;

		return temp;
	}



};




class triangle
{
public:

	triangle(void)
	{
		vertex[0].x = 0; vertex[0].y = 0; vertex[0].z = 0;
		vertex[1].x = 0; vertex[1].y = 0; vertex[1].z = 0;
		vertex[2].x = 0; vertex[2].y = 0; vertex[2].z = 0;
	}

	vertex_3 vertex[3];
};


void write_3fm_to_oss_buffer(vector<triangle> &triangles, ostringstream &buffer)
{
	buffer.str("");
	buffer.clear();


	cout << "Processing " << triangles.size() << " triangles..." << endl;

	// Insert unique vertices into set (may get rid of some near-duplicates)
	set<vertex_3> final_vertices;

	for (vector<triangle>::const_iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		final_vertices.insert((*i).vertex[0]);
		final_vertices.insert((*i).vertex[1]);
		final_vertices.insert((*i).vertex[2]);
	}

	// Add indexes to the vertices, helps speed up triangle / vertex lookup
	size_t index = 0;

	vector<vertex_3> temp_vertices;

	for (set<vertex_3>::iterator i = final_vertices.begin(); i != final_vertices.end(); i++)
	{
		temp_vertices.push_back(*i);
		temp_vertices[temp_vertices.size() - 1].index = index++;
	}

	final_vertices.clear();

	for (size_t i = 0; i < temp_vertices.size(); i++)
		final_vertices.insert(temp_vertices[i]);

	temp_vertices.clear();

	// Find 3 vertices for each triangle, adding the index to the triangle
	set<vertex_3>::const_iterator find_iter;

	// Assign vertex indices
	for (vector<triangle>::iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		find_iter = final_vertices.find((*i).vertex[0]);
		(*i).vertex[0].index = (*find_iter).index;

		find_iter = final_vertices.find((*i).vertex[1]);
		(*i).vertex[1].index = (*find_iter).index;

		find_iter = final_vertices.find((*i).vertex[2]);
		(*i).vertex[2].index = (*find_iter).index;
	}




	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
	buffer << "  <model unit=\"inch\" xmlns=\"http://schemas.microsoft.com/3dmanufacturing/2013/01\">" << endl;
	buffer << "    <resources>" << endl;
	buffer << "      <object id=\"0\" type=\"model\">" << endl;
	buffer << "        <mesh>" << endl;
	buffer << "          <vertices>" << endl;

	cout << "Writing " << final_vertices.size() << " vertices to memory buffer..." << endl;

	buffer << setiosflags(ios_base::fixed);
	buffer << setprecision(18);

	for (set<vertex_3>::const_iterator i = final_vertices.begin(); i != final_vertices.end(); i++)
		buffer << "<vertex x=\"" << (*i).x << "\" y=\"" << (*i).y << "\" z=\"" << (*i).z << "\" />" << endl;

	buffer << "          </vertices>" << endl;
	buffer << "          <triangles>" << endl;

	cout << "Writing " << triangles.size() << " triangles to memory buffer..." << endl;

	for (vector<triangle>::const_iterator i = triangles.begin(); i != triangles.end(); i++)
		buffer << "<triangle v1=\"" << (*i).vertex[0].index << "\" v2=\"" << (*i).vertex[1].index << "\" v3=\"" << (*i).vertex[2].index << "\" />" << endl;

	buffer << "          </triangles>" << endl;
	buffer << "        </mesh>" << endl;
	buffer << "      </object>" << endl;
	buffer << "    </resources>" << endl;
	buffer << "    <build>" << endl;
	buffer << "      <item objectid=\"0\" />" << endl;
	buffer << "    </build>" << endl;
	buffer << "  </model>" << endl;
}


bool load_from_stl(vector<triangle> &triangles, const char* const file_name)
{
	triangles.clear();

	// Read from file.
	ifstream in(file_name, ios_base::binary);

	if (in.fail())
		return false;

	const size_t header_size = 80;
	vector<char> buffer(header_size, 0);
	unsigned int num_triangles = 0; // Must be 4-byte unsigned int.

	// Read header.
	in.read(reinterpret_cast<char *>(&(buffer[0])), header_size);

	if (header_size != in.gcount())
		return false;

	// If ASCII text file header, abort
	if ('s' == tolower(buffer[0]) &&
		'o' == tolower(buffer[1]) &&
		'l' == tolower(buffer[2]) &&
		'i' == tolower(buffer[3]) &&
		'd' == tolower(buffer[4]))
	{
		cout << "Encountered ASCII STL file header -- aborting" << endl;
		return false;
	}



	// Read number of triangles.
	in.read(reinterpret_cast<char *>(&num_triangles), sizeof(unsigned int));

	if (sizeof(unsigned int) != in.gcount())
		return false;

	triangles.resize(num_triangles);

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t data_size = (12 * sizeof(float)+sizeof(short unsigned int)) * num_triangles;
	buffer.resize(data_size, 0);

	in.read(reinterpret_cast<char *>(&buffer[0]), data_size);

	if (data_size != in.gcount())
		return false;

	// Use a pointer to assist with the copying.
	// Should probably use std::copy() instead, but memcpy() does the trick, so whatever...
	char *cp = &buffer[0];

	for (vector<triangle>::iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		// Skip face normal.
		cp += 3 * sizeof(float);

		// Get vertices.
		memcpy(&i->vertex[0].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[0].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[0].z, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].z, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].z, cp, sizeof(float)); cp += sizeof(float);

		// Skip attribute.
		cp += sizeof(short unsigned int);
	}

	in.close();

	return true;
}

