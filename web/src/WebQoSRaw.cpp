/*
 * WebBiRawChart.cpp
# ------------------------------------------------------------------------ #
# Copyright (c) 2010-2015 Rodrigue Chakode (rodrigue.chakode@ngrt4n.com)   #
# Creation: 17-07-2015                                                     #
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

#include "WebQoSRaw.hpp"
#include <Wt/WStandardItemModel.h>
#include <Wt/WFont.h>


WebQoSRaw::WebQoSRaw(const std::string& vame)
  : Wt::Chart::WCartesianChart(),
    m_viewName(vame)
{
  auto model = std::make_unique<Wt::WStandardItemModel>();
  setModel(std::move(model));
  setStyleClass("bi-chart");
  setLegendEnabled(false);
  setType(Wt::Chart::ChartType::Scatter);
  axis(Wt::Chart::Axis::X).setScale(Wt::Chart::AxisScale::DateTime);
  setTitleFont(ngrt4n::chartTitleFont());
}


void WebQoSRaw::setChartTitle(void)
{
  setTitle(Q_TR("Trend of IT problems (%)"));
}

void WebQoSRaw::updateData(const QosDataList& data)
{
  auto model = std::make_unique<Wt::WStandardItemModel>(static_cast<int>(data.size()), 9);
  model->setHeaderData(0, Q_TR("Date/time"));
  model->setHeaderData(1, Q_TR("Status"));
  model->setHeaderData(2, Q_TR("% Normal"));
  model->setHeaderData(3, Q_TR("% Minor"));
  model->setHeaderData(4, Q_TR("% Major"));
  model->setHeaderData(5, Q_TR("% Critical"));
  model->setHeaderData(6, Q_TR("% Unknown"));
  model->setHeaderData(7, Q_TR("placeholder for 0% value"));
  model->setHeaderData(8, Q_TR("placeholder for 100% value"));

  int row = 0;
  for (const auto& entry : data) {
    Wt::WDateTime date;
    date.setTime_t(entry.timestamp);

    model->setData(row, 0, date);
    model->setData(row, 1, entry.status);

    float sev = entry.normal;
    model->setData(row, 2, sev);

    sev += entry.minor;
    model->setData(row, 3, sev);

    sev += entry.major;
    model->setData(row, 4, sev);

    sev += entry.critical;
    model->setData(row, 5, sev);

    sev += entry.unknown;
    model->setData(row, 6, sev);

    // placeholders
    model->setData(row, 7, 0.0);
    model->setData(row, 8, 100.0);
    ++row;
  }

  setModel(std::move(model));

  setXSeriesColumn(0);

  const QMap<int, int> SerieSeverities = {
    {0, ngrt4n::Unset},
    {1, ngrt4n::Unset},
    {2, ngrt4n::Normal},
    {3, ngrt4n::Minor},
    {4, ngrt4n::Major},
    {5, ngrt4n::Critical},
    {6, ngrt4n::Unknown},
    {7, ngrt4n::Unset},
    {8, ngrt4n::Unset},
  };

  for (int i = 8; i >= 2; --i) {
    auto serie = std::make_unique<Wt::Chart::WDataSeries>(i, Wt::Chart::SeriesType::Line);
    Wt::WColor color = ngrt4n::severityWColor(SerieSeverities[i]);
    serie->setPen(color);
    serie->setBrush(color);
    serie->setStacked(true);
    serie->setFillRange(Wt::Chart::FillRangeType::MinimumValue);
    addSeries(std::move(serie));
  }
  setChartTitle();
}