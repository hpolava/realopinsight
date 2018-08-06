/*
 * Parser.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2014 Rodrigue Chakode (rodrigue.chakode@gmail.com)    #
# Last Update : 23-03-2014                                                 #
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
#include "Base.hpp"
#include "Parser.hpp"
#include "utilsCore.hpp"
#include "ThresholdHelper.hpp"
#include "K8sHelper.hpp"
#include <QObject>
#include <QtXml>
#include <iostream>
#include <cassert>


Parser::Parser(CoreDataT* _cdata, int _parsingMode, const BaseSettings* settings)
  : m_cdata(_cdata),
    m_parsingMode(_parsingMode),
    m_settings(settings)
{
}


Parser::~Parser()
{
  QFile fileHandler;
  if (fileHandler.exists(m_plainFile)) {
    fileHandler.remove(m_plainFile);
  }
  if (fileHandler.exists(m_dotFile) && m_lastErrorMsg.isEmpty()) {
    fileHandler.remove(m_dotFile);
  }
  fileHandler.close();
}

int Parser::processRenderingData(void)
{
  fixupVisilityAndDependenciesGraph();
  saveCoordinatesFile();
  return computeCoordinates();
}

std::pair<int, QString> Parser::parse(const QString& viewFile)
{
  if (! m_cdata) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Parser cdata is null"));
  }

  m_cdata->clear();
  QDomDocument xmlDoc;
  QDomElement xmlRoot;
  QFile file(viewFile);
  if (! file.open(QIODevice::ReadOnly|QIODevice::Text)) {
    file.close();
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Unable to open the file %1").arg(viewFile));
  }

  if (! xmlDoc.setContent(&file)) {
    file.close();
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Error while parsing the file %1").arg(viewFile));
  }

  file.close(); // The content of the file is already in memory

  xmlRoot = xmlDoc.documentElement();
  m_cdata->monitor = static_cast<qint8>(xmlRoot.attribute("monitor").toInt());
  m_cdata->format_version = xmlRoot.attribute("compat").toDouble();

  QDomNodeList xmlNodes = xmlRoot.elementsByTagName("Service");

  if (m_cdata->monitor == MonitorT::Kubernetes) {
    return loadK8sNamespaceView(xmlNodes, *m_cdata);
  }

  qint32 xmlNodeCount = xmlNodes.size();
  for (qint32 nodeIndex = 0; nodeIndex < xmlNodeCount; ++nodeIndex) {
    QDomElement xmlNode = xmlNodes.item(nodeIndex).toElement();
    NodeT node;
    node.parent.clear();
    node.monitored = false;
    node.id = xmlNode.attribute("id").trimmed();
    node.sev = node.sev_prop = ngrt4n::Unknown;
    node.sev_crule = xmlNode.attribute("statusCalcRule").toInt();
    node.sev_prule = xmlNode.attribute("statusPropRule").toInt();
    node.icon = xmlNode.firstChildElement("Icon").text().trimmed();
    node.name = xmlNode.firstChildElement("Name").text().trimmed();
    node.description = xmlNode.firstChildElement("Description").text().trimmed();
    node.alarm_msg = xmlNode.firstChildElement("AlarmMsg").text().trimmed();
    node.notification_msg = xmlNode.firstChildElement("NotificationMsg").text().trimmed();
    node.child_nodes = xmlNode.firstChildElement("SubServices").text().trimmed();
    node.weight = (m_cdata->format_version >= 3.1) ? xmlNode.attribute("weight").toDouble() : ngrt4n::WEIGHT_UNIT;

    if (node.sev_crule == CalcRules::WeightedAverageWithThresholds) {
      QString thdata = xmlNode.firstChildElement("Thresholds").text().trimmed();
      node.thresholdLimits = ThresholdHelper::dataToList(thdata);
      qSort(node.thresholdLimits.begin(), node.thresholdLimits.end(), ThresholdLessthanFnt());
    }

    node.check.status = -1;
    if (node.icon.isEmpty()) {
      node.icon = ngrt4n::DEFAULT_ICON;
    }

    node.type = xmlNode.attribute("type").toInt();
    switch(node.type) {
      case NodeType::ITService:
        insertITServiceNode(node);
        break;
      case NodeType::BusinessService:
      case NodeType::ExternalService:
      default:
        m_cdata->bpnodes.insert(node.id, node);
        break;
    }
  }

  return std::make_pair(ngrt4n::RcSuccess, "");
}


std::pair<int, QString> Parser::loadK8sNamespaceView(QDomNodeList& in_xmlNodes, CoreDataT& out_cdata)
{
  if (in_xmlNodes.size() != 1) {
    return std::make_pair(ngrt4n::RcParseError, QObject::tr("Unexpected number of nodes in Kubernetes service file: %1").arg(in_xmlNodes.size()));
  }

  QDomElement xmlNode = in_xmlNodes.item(0).toElement();
  auto&& sourceId = xmlNode.attribute("id").trimmed();
  auto&& ns = xmlNode.firstChildElement("Name").text().trimmed();

  out_cdata.sources.insert(sourceId);

  SourceT sinfo;
  if (! m_settings->loadSource(sourceId, sinfo)) {
    return std::make_pair(ngrt4n::RcGenericFailure, QObject::tr("Failed loading Kubernetes data source: %1").arg(sourceId));
  }

  auto outK8sLoadNsView = K8sHelper(sinfo.mon_url, sinfo.verify_ssl_peer).loadNamespaceView(ns, out_cdata);
  if (outK8sLoadNsView.second != ngrt4n::RcSuccess) {
    auto&& m_lastErrorMsg = ! outK8sLoadNsView.first.isEmpty()? outK8sLoadNsView.first.at(0) : QObject::tr("Got weird error when load view from Kubernetes");
    return std::make_pair(outK8sLoadNsView.second, m_lastErrorMsg);
  }

  return std::make_pair(ngrt4n::RcSuccess, "");
}

QString Parser::escapeLabel(const QString& label)
{
  QString rwLabel = label;
  return rwLabel.replace("'", " ").replace("-", " ").replace("\"", " ").replace(' ', '#').replace(';', '_').replace('&', '_').replace('$', '_');
}

QString Parser::escapeId(const QString& id)
{
  QString rwId = id;
  return rwId.replace("'", " ").replace("-", "_").replace("\"", "_").replace(' ', '_').replace('#', '_');
}



void Parser::fixupVisilityAndDependenciesGraph(void)
{
  m_dotContent = "\n";

  for (auto&& bpnode:  m_cdata->bpnodes) {
    bpnode.visibility = ngrt4n::Visible|ngrt4n::Expanded;
    auto graphParentId = escapeId(bpnode.id);
    m_dotContent.insert(0, QString("\t%1[label=\"%2\"];\n").arg(graphParentId, escapeLabel(bpnode.name)));
    if (bpnode.type == NodeType::ExternalService) {
      continue;
    }

    if (! bpnode.child_nodes.isEmpty()) {
      QStringList children = bpnode.child_nodes.split(ngrt4n::CHILD_SEP.c_str());
      for(const auto& childId: children) {
        NodeListT::Iterator childIt;
        if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, childId, childIt)) {
          childIt->parent = bpnode.id;
          auto graphChildId = escapeId(childIt->id);
          m_dotContent.append(QString("\t%1--%2\n").arg(graphParentId, graphChildId));
        } else {
          qDebug() << QObject::tr("Failed to find parent-child dependency'%1' => %2").arg(bpnode.id, childId);
        }
      }
    }
  }

  // Set IT service nodes' labels
  for (auto&& cnode: m_cdata->cnodes) {
    cnode.visibility = ngrt4n::Visible;
    m_dotContent.insert(0, QString("\t%1[label=\"%2\"];\n").arg(escapeId(cnode.id), escapeLabel(cnode.name)));
  }

}


void Parser::saveCoordinatesFile(void)
{
  m_dotFile = QDir::tempPath()%"/realopinsight-gen-"%QTime().currentTime().toString("hhmmsszzz")%".dot";
  m_plainFile = m_dotFile % ".plain";
  QFile file(m_dotFile);
  if (! file.open(QIODevice::WriteOnly|QIODevice::Text)) {
    m_lastErrorMsg = QObject::tr("Unable into write file %1").arg(m_dotFile);
    file.close();
    exit(1);
  }
  QTextStream fstream(&file);
  fstream << "strict graph\n{\n node[shape=plaintext]\n"
          << m_dotContent
          << "}";
  file.close();
}

int Parser::computeCoordinates(void)
{
  QProcess process;
  QStringList arguments = QStringList() << "-Tplain"<< "-o" << m_plainFile << m_dotFile;

  int exitCode = -2;
  switch (m_settings->getGraphLayout()) {
    case ngrt4n::DotLayout:
      exitCode = process.execute("dot", arguments);
      break;
    case ngrt4n::NeatoLayout: //use dot as default
    default:
      exitCode = process.execute("neato", arguments);
      break;
  }

  process.waitForFinished(60000);
  if (exitCode != 0) {
    m_lastErrorMsg = QObject::tr("The graph engine exited on error (code: %1, file: %2").arg(QString::number(exitCode), m_dotFile);
    return ngrt4n::RcGenericFailure;
  }

  QFile qfile(m_plainFile);
  if (! qfile.open(QFile::ReadOnly)) {
    m_lastErrorMsg = QObject::tr("Failed to open file: %1").arg(m_plainFile);
    return ngrt4n::RcGenericFailure;
  }

  //start parsing
  QTextStream coodFileStream(& qfile);
  QString line;
  if(static_cast<void>(line = coodFileStream.readLine(0)), line.isNull()) {
    m_lastErrorMsg = QObject::tr("Failed to read file: %1").arg(m_plainFile);
    return ngrt4n::RcGenericFailure;
  }

  QRegExp regexSep("[ ]+");
  QStringList splitedLine = line.split (regexSep);
  if (splitedLine.size() != 4 || splitedLine[0] != "graph") {
    m_lastErrorMsg = QObject::tr("Invalid graphviz entry: %1").arg(line);
    return ngrt4n::RcGenericFailure;
  }

  const ScaleFactors SCALE_FACTORS(m_settings->getGraphLayout());
  m_cdata->graph_mode = static_cast<qint8>(m_settings->getGraphLayout());
  m_cdata->map_width = splitedLine[2].trimmed().toDouble() * SCALE_FACTORS.x();
  m_cdata->map_height = splitedLine[3].trimmed().toDouble() * SCALE_FACTORS.y();
  m_cdata->min_x = 0;
  m_cdata->min_y = 0;
  double max_text_w = 0;
  double max_text_h = 0;

  int x_index = 2;
  int y_index = 3;

  while (static_cast<void>(line = coodFileStream.readLine(0)), ! line.isNull()) {
    splitedLine = line.split (regexSep);
    if (splitedLine[0] == "node") {
      NodeListT::Iterator node;
      QString nid = splitedLine[1].trimmed();
      if (ngrt4n::findNode(m_cdata->bpnodes, m_cdata->cnodes, nid, node)) {
        node->pos_x = splitedLine[x_index].trimmed().toDouble() * SCALE_FACTORS.x();
        node->pos_y =  splitedLine[y_index].trimmed().toDouble() * SCALE_FACTORS.y();
        node->text_w = splitedLine[4].trimmed().toDouble() * SCALE_FACTORS.x();
        node->text_h = splitedLine[5].trimmed().toDouble() * SCALE_FACTORS.y();
        m_cdata->min_x = qMin<double>(m_cdata->min_x, node->pos_x);
        m_cdata->min_y = qMin<double>(m_cdata->min_y, node->pos_y);
        max_text_w = qMax(max_text_w, node->text_w);
        max_text_h = qMax(max_text_h, node->text_h);
      }
    } else if (splitedLine[0] == "edge") {
      // multiInsert because a node can have several childs
      m_cdata->edges.insertMulti(splitedLine[1], splitedLine[2]);
    } else if (splitedLine[0] == "stop") {
      break;
    }
  }

  qfile.close();

  if (m_settings->getGraphLayout() == ngrt4n::NeatoLayout) {
    m_cdata->min_x -= (max_text_w * 0.6);
    m_cdata->min_y -= (max_text_h * 0.6);
  }

  const double MAP_BORDER_HEIGHT = 50.0;
  const double MAP_BORDER_WIDTH = 200;

  m_cdata->min_x = qAbs(m_cdata->min_x) + MAP_BORDER_WIDTH;
  m_cdata->min_y = qAbs(m_cdata->min_y) + MAP_BORDER_HEIGHT;
  m_cdata->map_width += m_cdata->min_x;
  m_cdata->map_height += m_cdata->min_y;

  return ngrt4n::RcSuccess;
}


void Parser::insertITServiceNode(NodeT& node)
{
  StringPairT dataPointInfo = ngrt4n::splitDataPointInfo(node.child_nodes);
  m_cdata->hosts[dataPointInfo.first] << dataPointInfo.second;

  QString srcid = ngrt4n::getSourceIdFromStr(dataPointInfo.first);
  if (srcid.isEmpty()) {
    srcid = ngrt4n::sourceId(0);
    if (m_parsingMode == ParsingModeDashboard) {
      node.child_nodes = ngrt4n::realCheckId(srcid, node.child_nodes);
    }
  }
  m_cdata->sources.insert(srcid);
  m_cdata->cnodes.insert(node.id, node);
}




