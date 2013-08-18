//
// C++ Interface: exception
//
// Description: 
//
//
// Author: Thomas Michael Bohnert <tmb@nginet.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

class Exception {
	char *errMessage;
public:
	Exception(char* text);
	char *getMessage();
};

inline Exception::Exception(char* text){
	errMessage = text;
}

inline char *Exception::getMessage(){
	return errMessage;
}
