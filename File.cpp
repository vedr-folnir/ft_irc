#include "File.hpp"

File::File() {};
File::File( std::string Name, std::string Path, std::string Sender, std::string Receiver ): filename(filename), Path(Path), sender(Sender), receiver(Receiver) {};
File::File( const File &x ) { *this = x; };
File::~File() {};

File & File::operator=( const File &other ) {
	if (this == &other)
		return (*this);
	this->filename = other.filename;
	this->Path = other.Path;
	this->sender = other.sender;
	this->receiver = other.receiver;
	return (*this);
};