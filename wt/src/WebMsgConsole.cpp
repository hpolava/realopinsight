/*
 * WebMsgConsole.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2012 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Last Update : 19-09-2013                                                 #
#                                                                          #
# This file is part of RealOpInsight (http://RealOpInsight.com) authored   #
# by Rodrigue Chakode <rodrigue.chakode@gmail.com>                         #
#                                                                          #
# RealOpInsight is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by     #
# the Free Software Foundation, either version 3 of the License, or        #
# (at your option) any later version.                                      #
#                                                                          #
# The Software is distributed in the hope that it will be useful,          #
# but WITHOUT ANY WARRANTY; without even the implied warranty of           #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
# GNU General Public License for more details.                             #
#                                                                          #
# You should have received a copy of the GNU General Public License        #
# along with RealOpInsight.  If not, see <http://www.gnu.org/licenses/>.   #
#--------------------------------------------------------------------------#
 */

#include "WebMsgConsole.hpp"
#include "utilsClient.hpp"

WebMsgConsole::WebMsgConsole()
  : WTableView(0),
    m_rowCount(0)
{
  setSortingEnabled(true);
  setLayoutSizeAware(true);
  setColumnResizeEnabled(true);
  setAlternatingRowColors(true);
  setSelectable(true);
  setSelectionMode(Wt::SingleSelection);
  setSelectionBehavior(Wt::SelectRows);
  setHeaderHeight(26);

  m_model = new Wt::WStandardItemModel(2, 5);
  m_model->setHeaderData(0,
                         Wt::Horizontal,
                         QObject::tr("Date & Hour").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(1,
                         Wt::Horizontal,
                         QObject::tr("Severity").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(2,
                         Wt::Horizontal,
                         QObject::tr("Host").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(3,
                         Wt::Horizontal,
                         QObject::tr("Service").toStdString(),
                         Wt::DisplayRole);
  m_model->setHeaderData(4,
                         Wt::Horizontal,
                         QObject::tr("Message").toAscii(),
                         Wt::DisplayRole);

  SortingProxyModel* sproxy = new SortingProxyModel(this);
  sproxy->setSourceModel(m_model);
  sproxy->setDynamicSortFilter(true);
  sproxy->setFilterRole(Wt::UserRole);
  setModel(sproxy);

  sortByColumn(1, Wt::DescendingOrder);
}

WebMsgConsole::~WebMsgConsole()
{
  delete m_model;
}

void  WebMsgConsole::layoutSizeChanged(int width, int)
{
  Wt::WLength em = Wt::WLength(1, Wt::WLength::FontEx);
  setColumnWidth(0, 20 * em);
  setColumnWidth(1, 20 * em);
  setColumnWidth(2, 20 * em);
  setColumnWidth(3, 90); /*size of the header image*/
  setColumnWidth(4, width - (65 * em.toPixels() + 90)); /*size of the header image*/
}

void WebMsgConsole::update(const NodeListT& _cnodes)
{
  for(NodeListT::ConstIterator node=_cnodes.begin(), end=_cnodes.end();
      node != end; ++node)
  {
    addMsg(*node);
  }
}

void WebMsgConsole::addMsg(const NodeT&  _node)
{
  m_model->setItem(m_rowCount, 0, createDateTimeItem(_node.check.last_state_change));
  m_model->setItem(m_rowCount, 1, createStatusItem(_node));
  m_model->setItem(m_rowCount, 2, new Wt::WStandardItem(_node.check.host));
  m_model->setItem(m_rowCount, 3, new Wt::WStandardItem(_node.name.toStdString()));
  m_model->setItem(m_rowCount, 4, new Wt::WStandardItem(_node.actual_msg.toStdString()));

  ++m_rowCount;
}

Wt::WStandardItem* WebMsgConsole::createStatusItem(const NodeT& _node)
{
  Wt::WStandardItem * item = new Wt::WStandardItem();
  item->setData(std::string("3"), Wt::UserRole);
  item->setText(utils::severity2Str(_node.severity).toStdString());

  switch(_node.severity) {
    case MonitorBroker::Normal:
      item->setStyleClass("severity-normal");
      break;
    case MonitorBroker::Minor:
      item->setStyleClass("severity-minor");
      break;
    case MonitorBroker::Major:
      item->setStyleClass("severity-major");
      break;
    case MonitorBroker::Critical:
      item->setStyleClass("severity-critical");
      break;
    case MonitorBroker::Unknown:
    default:
      item->setStyleClass("severity-unknown");
      break;
  }
  return item;
}

Wt::WStandardItem* WebMsgConsole::createDateTimeItem(const std::string& _lastcheck)
{
  Wt::WStandardItem * item = new Wt::WStandardItem();
  long time = atoi(_lastcheck.c_str());
  item->setText( ctime(&time) );
  item->setData(_lastcheck, Wt::UserRole);
  return item;
}
