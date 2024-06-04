#include "FileUpload.hpp"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

MultipartFormData FileUploader::parse_multipart_form_data(const std::string &boundary, const std::string &body)
{
	MultipartFormData formData;
	std::string boundaryMarker = "--" + boundary;
	std::string endBoundaryMarker = "--" + boundary + "--";

	size_t pos = body.find(boundaryMarker);
	size_t endPos = body.find(endBoundaryMarker);

	if (pos == std::string::npos || endPos == std::string::npos) {
		return formData;
	}

	pos += boundaryMarker.size() + 2; // Move past boundary and \r\n
	std::string part = body.substr(pos, endPos - pos);

	size_t headerEnd = part.find("\r\n\r\n");
	if (headerEnd != std::string::npos) {
		std::string headers = part.substr(0, headerEnd);
		std::string content = part.substr(headerEnd + 4);

		size_t fileNamePos = headers.find("filename=\"");
		if (fileNamePos != std::string::npos) {
			fileNamePos += 10;
			size_t fileNameEnd = headers.find("\"", fileNamePos);
			formData.fileName = headers.substr(fileNamePos, fileNameEnd - fileNamePos);

			size_t contentEnd = content.rfind("\r\n--");
			if (contentEnd != std::string::npos) {
				formData.fileContent.assign(content.begin(), content.begin() + contentEnd);
			}
			else {
				formData.fileContent.assign(content.begin(), content.end());
			}
		}
	}

	return formData;
}

FileUploader::FileUploader()
{
}

FileUploader::~FileUploader()
{
}
