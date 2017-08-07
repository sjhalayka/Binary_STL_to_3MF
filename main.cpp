#include "main.h"
#include "miniz.h"


int main(int argc, const char **argv)
{
	if(3 != argc)
	{
		cout << "Usage: " << argv[0] << " input.stl output.3mf" << endl;
		return -1;
	}

	cout << "Loading from binary STL file " << argv[1] << endl;
	vector<triangle> triangles;

	if (false == load_from_stl(triangles, argv[1]))
	{
		cout << "Could not load from binary STL file " << argv[1] << endl;
		return -2;
	}
	else
	{
		cout << "Success" << endl;
	}

	if (0 == triangles.size())
	{
		cout << "No triangles found in binary STL file" << endl;
		return -3;
	}


	string archive_file_name = argv[2];
	string comment = "";
	remove(archive_file_name.c_str());


	cout << "Creating directory _rels/" << endl;

	if (1 == mz_zip_add_mem_to_archive_file_in_place(archive_file_name.c_str(), "_rels/", NULL, 0, comment.c_str(), comment.size(), MZ_BEST_COMPRESSION))
	{
		cout << "Success" << endl;
	}
	else
	{
		cout << "Failure" << endl;
		return -4;
	}




	cout << "Creating directory 3D/" << endl;

	if(1 == mz_zip_add_mem_to_archive_file_in_place(archive_file_name.c_str(), "3D/", NULL, 0, comment.c_str(), comment.size(), MZ_BEST_COMPRESSION))
	{
		cout << "Success" << endl;
	}
	else
	{
		cout << "Failure" << endl;
		return -5;
	}




	cout << "Writing [Content_Types].xml file" << endl;
		
	ostringstream buffer;

	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
	buffer << "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" << endl;
	buffer << "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\" />" << endl;
	buffer << "<Default Extension=\"model\" ContentType=\"application/vnd.ms-package.3dmanufacturing-3dmodel+xml\" />" << endl;
	buffer << "</Types>" << endl;

	if (1 == mz_zip_add_mem_to_archive_file_in_place(archive_file_name.c_str(), "[Content_Types].xml", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION))
	{
		cout << "Success" << endl;
	}
	else
	{
		cout << "Failure" << endl;
		return -6;
	}
	


	cout << "Writing _rels/.rels file" << endl;

	buffer.str("");
	buffer.clear();

	buffer << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
	buffer << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" << endl;
	buffer << "<Relationship Target=\"/3D/3dmodel.model\" Id=\"rel0\" Type=\"http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel\" />" << endl;
	buffer << "</Relationships>" << endl;

	if(1 == mz_zip_add_mem_to_archive_file_in_place(archive_file_name.c_str(), "_rels/.rels", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION))
	{
		cout << "Success" << endl;
	}
	else
	{
		cout << "Failure" << endl;
		return -7;
	}



	cout << "Writing 3D/3dmodel.model file" << endl;

	write_3fm_to_oss_buffer(triangles, buffer);

	if (1 == mz_zip_add_mem_to_archive_file_in_place(archive_file_name.c_str(), "3D/3dmodel.model", buffer.str().c_str(), buffer.str().size(), comment.c_str(), comment.size(), MZ_BEST_COMPRESSION))
	{
		cout << "Success" << endl;
	}
	else
	{
		cout << "Failure" << endl;
		return -8;
	}
	


	return 0;
}