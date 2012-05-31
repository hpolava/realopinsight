TEMPLATE = app
TARGET = build/usr/bin/ngrt4n
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc

QT        += core gui xml svg webkit 

INCLUDEPATH = include  include/client include/core 

HEADERS   += include/core/ns.hpp \
			 include/core/MonitorBroker.hpp \
			 include/client/Auth.hpp \
			 include/client/Base.hpp \
			 include/client/GraphView.hpp \
			 include/client/MsgPanel.hpp \
			 include/client/Parser.hpp \
			 include/client/PieChart.hpp \
			 include/client/PieChartItem.hpp \
			 include/client/Preferences.hpp \
			 include/client/ServiceEditor.hpp \
			 include/client/Stats.hpp \
			 include/client/StatsLegend.hpp \
			 include/client/SvConfigCreator.hpp \
			 include/client/SvNavigator.hpp \
			 include/client/SvNavigatorTree.hpp \
			 include/client/WebKit.hpp 
			 
SOURCES   += src/client/ngrt4n.cpp \
			 src/core/utils.cpp \
			src/core/MonitorBroker.cpp \
			src/client/Auth.cpp \
			src/client/Base.cpp \
			src/client/GraphView.cpp \
			src/client/MsgPanel.cpp \ 
            src/client/Parser.cpp \
            src/client/PieChart.cpp \
            src/client/PieChartItem.cpp \
            src/client/Preferences.cpp \
            src/client/ServiceEditor.cpp \
            src/client/Stats.cpp \
            src/client/StatsLegend.cpp \
            src/client/SvConfigCreator.cpp \
            src/client/SvNavigator.cpp \
            src/client/SvNavigatorTree.cpp \
            src/client/WebKit.cpp 
            
LIBS += -lcrypt  -L/usr/local/lib64 -lxmlrpc_client++

FORMS	  +=
RESOURCES += ngrt4n.qrc
