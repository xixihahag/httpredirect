////
// @file SocketAddressUn.cc
// @brief
// unixsocketaddress'shixain
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#include <string.h>
#include <cstring>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ndsl/net/SocketAddressUn.h"

using namespace std;

namespace ndsl{
namespace net{

SocketAddressUn::SocketAddressUn()
{
	sun_family = AF_LOCAL;
	memset( sun_path, 0, strlen(sun_path) );
}

SocketAddressUn::SocketAddressUn( const string& path)
{
	string p = path;
	memset( sun_path, 0, strlen(sun_path) );
	sun_family = AF_LOCAL;
	memcpy( sun_path, p.c_str(), p.length() + 1 );
}

SocketAddressUn::SocketAddressUn( const SocketAddressUn& addr)
{
	sun_family = addr.sun_family;
	memcpy( sun_path, addr.sun_path, strlen(addr.sun_path) );
}

SocketAddressUn::~SocketAddressUn(){}

SocketAddressUn& SocketAddressUn::operator=( const SocketAddressUn& addr )
{
	sun_family = addr.sun_family;
	memcpy( sun_path, addr.sun_path, strlen(addr.sun_path) );

	return *this;
}

void SocketAddressUn::setAddress( const string& path )
{
	string p = path;
	memset( sun_path, 0, strlen(sun_path) );
	memcpy( sun_path, p.c_str(), p.length() + 1 );
}

void SocketAddressUn::setAddress( const SocketAddressUn& addr )
{
	sun_family = addr.sun_family;
	memcpy( sun_path, addr.sun_path, strlen(addr.sun_path) );
}

string SocketAddressUn::getStrPath()
{
	return sun_path;
}

}	// namespace net
}	// namespace ndsl

