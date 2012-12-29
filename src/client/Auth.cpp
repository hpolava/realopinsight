/*
 * Auth.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update: 24-05-2012                                                 #
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


#include "Auth.hpp"
#include "Preferences.hpp"
#include "ns.hpp"
#include "utilsClient.hpp"

const QString Auth::ADM_USER_NAME = USER_BASE_NAME%"_adm" ;
const QString Auth::OP_USER_NAME = USER_BASE_NAME%"_op" ;
const qint32 Auth::ADM_USER_ROLE = 100 ;
const qint32 Auth::OP_USER_ROLE = 101 ;

Auth::Auth()
  : QDialog(),
    settings (new Settings())
{
  setWindowTitle(tr("%1 - Login").arg(appName));
  layout = new QGridLayout(this);
  qint32 line = 0 ;
  QPixmap logo(":images/built-in/logo.png") ;
  QLabel* llogo =  new QLabel(); llogo->setPixmap(logo) ;
  layout->addWidget(llogo, line, 0, 1, 3, Qt::AlignLeft) ;
  line++ ;
  layout->addWidget(new QLabel(tr("Version %1 (%2)").arg(packageVersion).arg(releaseName)), line, 0, 2, 1, Qt::AlignLeft) ;
  line++;
  layout->addWidget(new QLabel(tr("Login")), line, 1, Qt::AlignRight) ;
  layout->addWidget(login = new QLineEdit(OP_USER_NAME), line, 2, Qt::AlignJustify);
  line++;
  layout->addWidget(new QLabel(tr("Password")), line, 1, Qt::AlignRight) ;
  layout->addWidget(password = new QLineEdit(), line, 2, Qt::AlignJustify);
  password->setEchoMode(QLineEdit::Password);
  line++;
  layout->addWidget(buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok), line, 1, 1, 3, Qt::AlignRight);
  line++; QString copying = QString("\nCopyright (c) 2010-"+releaseYear +" by NGRT4N Project. All rights reserved.");
  layout->addWidget(new QLabel(copying), line, 0, 1, 3, Qt::AlignLeft) ;
  addEvents();
}

Auth::~Auth()
{
  delete login ;
  delete password ;
  delete layout ;
}


void Auth::cancel(void)
{
  exit( 1 );
}


void Auth::authentificate(void)
{
  QString userName = login->text() ;
  QString userPasswd = QCryptographicHash::hash(password->text().toAscii(), QCryptographicHash::Md5) ;
  QString rootPasswd =  settings->value(Preferences::ADM_PASSWD_KEY).toString() ;
  QString opPasswd =  settings->value(Preferences::OP_PASSWD_KEY).toString() ;
  if(	! rootPasswd.isEmpty()
        && userName == ADM_USER_NAME
        && userPasswd == rootPasswd ) {
      done(ADM_USER_ROLE);
    } else if( !opPasswd.isEmpty()
               && userName == OP_USER_NAME
               && userPasswd == opPasswd ) {
      done(OP_USER_ROLE);
    } else {
      utils::alert(tr("Authentication failed: wrong username or password"));
    }
}


void Auth::addEvents(void)
{
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(authentificate()));
}
