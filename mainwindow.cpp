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

/************************************************************************************************************
**                                                                                                         **
**  This is the example code for QCustomPlot.                                                              **
**                                                                                                         **
**  It demonstrates basic and some advanced capabilities of the widget. The interesting code is inside     **
**  the "setup(...)Demo" functions of MainWindow.                                                          **
**                                                                                                         **
**  In order to see a demo in action, call the respective "setup(...)Demo" function inside the             **
**  MainWindow constructor. Alternatively you may call setupDemo(i) where i is the index of the demo       **
**  you want (for those, see MainWindow constructor comments). All other functions here are merely a       **
**  way to easily create screenshots of all demos for the website. I.e. a timer is set to successively     **
**  setup all the demos and make a screenshot of the window area and save it in the ./screenshots          **
**  directory.                                                                                             **
**                                                                                                         **
*************************************************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

MainWindow::MainWindow(const char * inputFile, uint32_t delayMilliSeconds, QWidget * parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_inputFile(inputFile),
  m_delayMilliSeconds(delayMilliSeconds),
  m_buffer(10)
{
  ui->setupUi(this);
  setGeometry(400, 250, 840, 480); // (.., .., width, height)
  
  setupDemo();
}

void MainWindow::setupDemo()
{
  setupSpectrumDemo(ui->customPlot);
  setWindowTitle("QCustomPlot: "+demoName);
  statusBar()->clearMessage();
  ui->customPlot->replot();
}

void MainWindow::addNextScan()
{
  Buffer * inBuffer = NULL;
  if (this->m_dataReader->GetNext(inBuffer) == 0) {
    for (uint32_t i = 0; i < inBuffer->size(); i++) {
      this->m_buffer.appendPoint(inBuffer->m_frequencyBuffer[i], inBuffer->m_powerBuffer[i]);
    }
  } else {
    dataTimer.stop();
    double milliSeconds = QDateTime::currentDateTime().toMSecsSinceEpoch();
    ui->statusBar->showMessage(
          QString("Done: %1 --> %2 scans/sec, Total scans: %3")
          .arg(QString(this->m_timeBuffer))
          .arg(this->m_scanCount*1000/(milliSeconds - this->m_startMilliSeconds), 0, 'f', 0)
          .arg(this->m_nextScanIndex)
          , 0);
    return;
  }

  ui->customPlot->graph()->clearData();
  double lowerFrequency = std::numeric_limits<double>::max();
  double upperFrequency = std::numeric_limits<double>::min();
  double lowerPower = std::numeric_limits<double>::max();
  double upperPower = std::numeric_limits<double>::min();
  int alpha = 27;
  for (CircularBuffer::BufferType * buffer : this->m_buffer.getBuffers()) {
    for (uint32_t i = 0; i < buffer->size(); i++) {
#if 0
      QPen pen;
      pen.setColor(QColor(0, 200, 0, alpha));
      ui->customPlot->graph()->setPen(pen);
#endif
      ui->customPlot->graph()->addData((*buffer)[i].first, (*buffer)[i].second);
      lowerFrequency = std::min<double>(lowerFrequency, (*buffer)[i].first);
      upperFrequency = std::max<double>(upperFrequency, (*buffer)[i].first);
      lowerPower = std::min<double>(lowerPower, (*buffer)[i].second);
      upperPower = std::max<double>(upperPower, (*buffer)[i].second);
    }
    alpha += 23;
  }

  ui->customPlot->xAxis->setRange(lowerFrequency - 50e6, upperFrequency + 50e6);
  ui->customPlot->yAxis->setRange(lowerPower - 0.1, upperPower + 0.3);
  bool expandOnly = this->m_nextScanIndex % 100 != 0;
  ui->customPlot->graph()->rescaleAxes(expandOnly);
  ui->customPlot->replot();
  double milliSeconds = QDateTime::currentDateTime().toMSecsSinceEpoch();
  this->m_scanCount++;
  if (milliSeconds - this->m_startMilliSeconds > 1000) {
    DataReader::TimeToString(inBuffer->m_time, 
                             this->m_timeBuffer, 
                             std::extent<decltype(this->m_timeBuffer)>::value);
    ui->statusBar->showMessage(
          QString("%1 --> %2 scans/sec, Total scans: %3")
          .arg(QString(this->m_timeBuffer))
          .arg(this->m_scanCount*1000/(milliSeconds - this->m_startMilliSeconds), 0, 'f', 0)
          .arg(this->m_nextScanIndex)
          , 0);
    this->m_startMilliSeconds = milliSeconds;
    this->m_scanCount = 0;
  }
  this->m_buffer.nextBuffer();
  this->m_nextScanIndex++;
}

void MainWindow::setupSpectrumDemo(QCustomPlot *customPlot)
{
  demoName = "Spectrum Demo";
  customPlot->legend->setVisible(true);
  customPlot->legend->setFont(QFont("Helvetica", 9));

  // Open the input file and create a reader.
  this->m_dataReader = new DataReader(this->m_inputFile);
  this->m_nextScanIndex = 0;

  QPen pen;
  //QStringList lineNames;
  // lineNames << "lsNone" << "lsLine" << "lsStepLeft" << "lsStepRight" << "lsStepCenter" << "lsImpulse";
  // add graphs with different line styles:
  QCPGraph::LineStyle lineStyle = QCPGraph::lsNone;
  customPlot->addGraph();
  pen.setColor(QColor(0, 200, 0));
  customPlot->graph()->setPen(pen);
  customPlot->graph()->setName("Power");
  customPlot->graph()->setLineStyle(lineStyle);
  customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
  // generate data:
  this->m_startMilliSeconds = QDateTime::currentDateTime().toMSecsSinceEpoch();
  this->m_scanCount = 0;
  this->addNextScan();
  // zoom out a bit:
  // ui->customPlot->yAxis->scaleRange(1.1, ui->customPlot->yAxis->range().center());
  // ui->customPlot->xAxis->scaleRange(1.1, ui->customPlot->xAxis->range().center());
  // set blank axis lines:
  customPlot->xAxis->setTicks(true);
  customPlot->yAxis->setTicks(true);
  customPlot->xAxis->setTickLabels(true);
  customPlot->yAxis->setTickLabels(true);
  // make top right axes clones of bottom left axes:
  customPlot->axisRect()->setupFullAxesBox();

  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(addNextScan()));
  dataTimer.start(this->m_delayMilliSeconds); // Interval 0 means to refresh as fast as possible
}

void MainWindow::setupPlayground(QCustomPlot *customPlot)
{
  Q_UNUSED(customPlot)
}

MainWindow::~MainWindow()
{
  delete ui;
}






































