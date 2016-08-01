/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011-2015 Emanuel Eichhammer                            **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 22.12.15                                             **
**          Version: 1.3.2                                                **
****************************************************************************/

#include <QApplication>
#include <QCommandLineParser>
#include <string>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QApplication::setGraphicsSystem("raster");
#endif

  QApplication a(argc, argv);
  QCoreApplication::setApplicationName("fastPlot");
  QCoreApplication::setApplicationVersion("1.0");
  QCommandLineParser parser;
  parser.setApplicationDescription("Program to plot output of scanner");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("<input>", QCoreApplication::translate("main", "Input file."));

  // An option with a value
  QCommandLineOption delayOption(QStringList() << "d" << "delay",
                                 QCoreApplication::translate("main", "Set the delay to plot each scan."),
                                 QCoreApplication::translate("main", "delay in ms"));
  parser.addOption(delayOption);

  // Process the actual command line arguments given by the user
  parser.process(a);

  const QStringList args = parser.positionalArguments();

  // source is args.at(0), destination is args.at(1)
  if (args.size() == 0) {
    parser.showHelp();
  }

  QString inputFile = args.at(0);
  uint32_t delay = 0;
  if (parser.value(delayOption) != QString("")) {
    delay = parser.value(delayOption).toUInt();
  }

  MainWindow w(inputFile.toLatin1().data(), delay);
  w.show();
  
  return a.exec();
}
