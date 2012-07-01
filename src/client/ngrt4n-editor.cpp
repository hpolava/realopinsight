/*
 * ngrt4n.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 24-05-2012                                                 #
#                                                                          #
# This file is part of NGRT4N (http://ngrt4n.com).                         #
#                                                                          #
# NGRT4N is free software: you can redistribute it and/or modify           #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# NGRT4N is distributed in the hope that it will be useful,                #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with NGRT4N.  If not, see <http://www.gnu.org/licenses/>.          #
#--------------------------------------------------------------------------#
 */

#include "core/ns.hpp"
#include "client/Auth.hpp"
#include "client/SvConfigCreator.hpp"
#include <sstream>


const string appName = APPLICATION_NAME ;
const string releaseYear = RELEASE_YEAR;
const string packageName = PACKAGE_NAME ;
const string packageVersion = PACKAGE_VERSION;
const string packageUrl = PACKAGE_URL;

QString  usage = "usage: " + QString(packageName.c_str()) + " [OPTION] [view_config]\n"
		"Options: \n"
		"	-v\n"
		"	  Prints the version and license information.\n"
		"	-h \n"
		"	   Prints this help" ;


ostringstream versionMsg(appName + " Editor, version " + packageVersion + ".\n"
		"Copyright (c) 2010-" + releaseYear + " NGRT4N Project <contact@ngrt4n.com>." + "\n"
		+"Visit "+ packageUrl + " for further information.") ;

int main(int argc, char **argv)
{
	QApplication* app = new QApplication(argc, argv) ;
	QIcon app_icon (":images/built-in/icon.png") ;
	app->setWindowIcon( app_icon ) ;
	app->setApplicationName(  QString(appName.c_str()) ) ;
	app->setStyleSheet(Preferences::style());

	if(argc > 3) {
		qDebug() << usage ;
		exit (1) ;
	}
	ngrt4n::initApp() ;
	QString file = argv[1] ;
	int opt ;

	if ( (opt = getopt(argc, argv, "hv") ) != -1) {
		switch (opt) {

		case 'v': {
			qDebug() << QString::fromStdString(versionMsg.str()) << endl;
			exit(0) ;
		}

		case 'h': {
			qDebug() << usage ;
			exit(0) ;
		}

		default: // -h for get help
			qDebug() << usage ;
			exit (1) ;
			break ;
		}
	}
	qDebug() << "Launching..." << endl
			<< QString::fromStdString(versionMsg.str()) << endl;

	Auth authentification;
	int userRole = authentification.exec() ;
	if( userRole != Auth::ADM_USER_ROLE && userRole != Auth::OP_USER_ROLE ) exit( 1 ) ;

	SvCreator* svc = new SvCreator(userRole) ;
	svc->load(file) ;

	return app->exec() ;
}
