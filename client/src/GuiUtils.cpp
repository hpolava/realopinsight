#include "GuiUtils.hpp"
#include "utilsClient.hpp"
#include "StatsLegend.hpp"

QSplashScreen* ngrt4n::infoScreen(const QString & msg) {
  QSplashScreen* screen = new QSplashScreen(QPixmap(":/images/built-in/loading-screen.png"));
  screen->showMessage(msg, Qt::AlignJustify|Qt::AlignVCenter);
  screen->show();
  return screen;
}

QColor ngrt4n::severityQColor(const int& _criticity)
{
  QColor color(ngrt4n::COLOR_UNKNOWN);
  switch (static_cast<ngrt4n::SeverityT>(_criticity)) {
    case ngrt4n::Normal:
      color = ngrt4n::COLOR_NORMAL;
      break;
    case ngrt4n::Minor:
      color = ngrt4n::COLOR_MINOR;
      break;
    case ngrt4n::Major:
      color = ngrt4n::COLOR_MAJOR;
      break;
    case ngrt4n::Critical:
      color = ngrt4n::COLOR_CRITICAL;
      break;
    default:
      break;
  }
  return color;
}

void ngrt4n::alert(const QString& msg)
{
  QMessageBox::warning(0, QObject::tr("%1 - Warning").arg(APP_NAME), msg, QMessageBox::Yes);
}

QIcon ngrt4n::severityIcon(int _severity)
{
  return QIcon(":/"+ngrt4n::getIconPath(_severity));
}

QString ngrt4n::getWelcomeMsg(const QString& utility)
{
  return QObject::tr("       > %1 %2 %3 (codename: %4)"
                     "\n        >> Realease ID: %5"
                     "\n        >> Copyright (C) 2010 - %6 RealOpInsight Labs. All rights reserved"
                     "\n        >> For bug reporting instructions, see: <%7>").arg(APP_NAME,
                                                                                   utility,
                                                                                   PKG_VERSION,
                                                                                   REL_NAME,
                                                                                   REL_INFO,
                                                                                   REL_YEAR,
                                                                                   PKG_URL);
}
