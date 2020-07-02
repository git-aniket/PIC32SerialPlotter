#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFont>
#include <QDebug>
#include <QtWidgets>
#include <qcustomplot.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    device =new QSerialPort;
    device_is_available=false;
    device_port_name="";
    serialBuffer="";


    //Configure fonts to be used
    QFont font;
    font.setPointSize(10);
    font.setItalic(true);
    font.setBold(true);


    //Add graph and set line color to the plot
    ui->CustomPlot->addGraph();
    ui->CustomPlot->graph(0)->setPen(QPen(Qt::red));
    ui->CustomPlot->graph(0)->setAntialiasedFill(false);

    ui->CustomPlot->addGraph();
    ui->CustomPlot->graph(1)->setAntialiasedFill(false);
    ui->CustomPlot->graph(1)->setPen(QPen(Qt::blue));

    //Configure X-Axis to time in secs
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->CustomPlot->xAxis->setTicker(timeTicker);
    ui->CustomPlot->axisRect()->setupFullAxesBox();

    //Configure x and y-Axis to display Labels
    ui->CustomPlot->xAxis->setTickLabelFont(QFont(QFont().family(),8));
    ui->CustomPlot->yAxis->setTickLabelFont(QFont(QFont().family(),8));
    ui->CustomPlot->xAxis->setLabel("Index");
    ui->CustomPlot->yAxis->setLabel("Magnitude");

    //Make top and right axis visible, but without ticks and label
    ui->CustomPlot->xAxis->setVisible(true);
    ui->CustomPlot->yAxis->setVisible(true);
    ui->CustomPlot->xAxis2->setVisible(true);
    ui->CustomPlot->yAxis2->setVisible(true);
    ui->CustomPlot->xAxis->setTicks(true);
    ui->CustomPlot->yAxis->setTicks(true);
    ui->CustomPlot->xAxis2->setTickLabels(false);
    ui->CustomPlot->yAxis2->setTickLabels(false);


    /*
    //Check the serial devices available
    qDebug()<<"Number of available ports: "<< QSerialPortInfo::availablePorts().length();

    //Check for vendor and product ID of each serial device
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        qDebug()<<"Has vendor ID: "<<serialPortInfo.hasVendorIdentifier();
        if(serialPortInfo.hasVendorIdentifier())
            qDebug()<<"Vendor ID: "<<serialPortInfo.vendorIdentifier();

        qDebug()<<"Has product ID: "<<serialPortInfo.hasProductIdentifier();
        if(serialPortInfo.hasProductIdentifier())
            qDebug()<<"Product ID: "<<serialPortInfo.productIdentifier();

    }
    */

        //check if the requisite port is available
        foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
        {
            if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
            {
                if(serialPortInfo.vendorIdentifier()==device_vendor_id)
                {
                    if(serialPortInfo.productIdentifier()==device_product_id)
                    {
                        device_port_name=serialPortInfo.portName();//get the port name of the port
                        device_is_available=true;//set the boolean to be used later

                    }
                }
            }

        }

        if(device_is_available)
        {
            //open and configure the port
            device->setPortName(device_port_name);
            device->open(QSerialPort::ReadWrite);
            device->setBaudRate(QSerialPort::Baud38400);
            device->setDataBits(QSerialPort::Data8);
            device->setParity(QSerialPort::NoParity);
            device->setStopBits(QSerialPort::OneStop);
            device->setFlowControl(QSerialPort::NoFlowControl);

            connect(device, SIGNAL(readyRead()), this , SLOT(store_data()));
        }

        else
        {
            //give error message
            QMessageBox::warning(this,"Port Error","Couldn't find the device");
            MainWindow::~MainWindow();//Call the destructor
        }


}


//Destructor
MainWindow::~MainWindow()
{
    //if the port is open, Close it before quitting
    if(device->isOpen())
    {
        device->close();
    }
    delete ui;
}

void MainWindow::on_btn_open_port_clicked()
{
        MainWindow::write_serial("\n");
}

void MainWindow::write_serial(QString command )
{
    if(device->isWritable())
    {
        device->write(command.toStdString().c_str());
        //qDebug()<<"Wrote to serial!";
        QMessageBox::information(this,"Info","Port Opened");
    }
    else
    {
        //qDebug()<<"Couldn't write to serial!";
        QMessageBox::warning(this,"Error!","Couldn't write to device.");
    }
}

void MainWindow::store_data()
{

     serialData+=device->readAll();// Read all serial data into a QByteArray

     //qDebug()<<"SerialData+endTemp"<<serialData;

     int len=serialData.length();

     int counter=0;

     //Iterate of the length starting from rear and check if termination character has been received
     for(int i=len-1;i>=0;--i)
     {
         //Check if the character matches the termination character
         if(serialData[i]=='\n')
         {
             ++counter;

             //If the number of newline characters match the numnber of plots
             if(counter==PLOT_COUNT)
             {

                 begin_temp="";
                 end_temp="";


                 for(int y=0;y<=i;++y)
                     begin_temp.append(serialData[y]);

                 for(int x=i+1;x<len;++x)
                     end_temp.append(serialData[x]);

                 //qDebug()<<"EndTemp"<<end_temp;
                 //qDebug()<<"BeginTemp"<<begin_temp<<"\n";
                 //qDebug()<<"EndTemp"<<end_temp<<"\n";


                 //If begin_temp is not the sole termination character
                 if(begin_temp.length()>2*PLOT_COUNT)
                 {
                     //plot the data
                     segregate_data(begin_temp.toStdString());
                     plot_data(data_index,data_point1,1);
                 }

                 serialData=end_temp;
                 break;

             }

         }

     }

}


/*Function to plot the string of data*/
void MainWindow::segregate_data(std::string plot_string)
{

    //convert std::string to QString
    QString data=QString::fromStdString(plot_string);

    //split the string by choice of delimiter
    QStringList data_split=data.split("\r\n");
    int l =data_split.length()-1;
    //qDebug()<<"DataSplit:"<<data_split;


    //Extract the meaningful data and remove the garbage values
    for(int i=0;i<l-1;++i)
    {

        //qDebug()<<i<<"-"<<data_split[i];
        if(counter<BUFF_LEN)
        {
            ++counter;
            data_index.append(counter);
            data_point1.append(data_split[i].toDouble());
        }
        if(counter>=BUFF_LEN)
        {
            ++counter;
            data_index.pop_front();
            data_index.append(counter);
            data_point1.pop_front();
            data_point1.append(data_split[i].toDouble());
        }
        //qDebug()<<data_point1;
        //qDebug()<<data_index;
    }

}


void MainWindow::plot_data(QVector<double> x, QVector<double> y, uint8_t plot_number)
{

    //qDebug()<<"Begin "<<x.at(0);
    //qDebug()<<"End "<<x.at(x.size()-1);
    ui->CustomPlot->graph(plot_number)->setData(x,y);
    ui->CustomPlot->graph(plot_number)->rescaleValueAxis();
    //ui->CustomPlot->xAxis->setRange(x[0],x.end(), Qt::AlignRight);
    ui->CustomPlot->xAxis->setRange(x.at(0),x.at(x.size()-1));
    ui->CustomPlot->yAxis->setRange(-10,4096);
    ui->CustomPlot->replot();

}


/*



//Function return's the first index of a char in a char array
int MainWindow::first_index_of(char* arr,int arrlen, char S)
{
    int first_index=-1;
    for(int i=0;i<arrlen;++i)
    {
        if(arr[i]==S)
        {
            first_index=i;
            break;
        }
    }

    return first_index;
}

//Function return's the last index of a char in a char array
int MainWindow::last_index_of(char* arr, int arrlen, char E)
{
    int last_index=-1;
    for(int i=arrlen-1;i>=0;--i)
    {
        if(arr[i]==E)
        {
            last_index=i;
            break;
        }
    }
    return last_index;
}



*/

/*
std::string MainWindow::read_data(char *arr)
{
    //start partitioning of data
    first_index=first_index_of(arr,BUFF_LEN,'S');
    last_index=last_index_of(arr,BUFF_LEN,'E');

    middle_string=arr;
    end_string=arr;

    //get the middle string and end string from the data_array
    middle_string=middle_string.substr(first_index,last_index-first_index+1);
    end_string=end_string.substr(last_index,BUFF_LEN-last_index);
    qDebug()<<QString::fromStdString(end_string);

    return middle_string;
}
*/


/*
void test()
{

QStringList bufferSplit=serialBuffer.split("\r\n");//Split the serialBuffer to get a list of stringys
int l=bufferSplit.length();//get the length of the buffer

if(l<=PLOT_COUNT)//if length of buffer is < no of plots then read again
{
    serialData=device->readAll();// Read all serial data into a QByteArray
    serialBuffer+=QString::fromStdString(serialData.toStdString());//Convert the QByte array into a QString
    qDebug()<<"Buffer Smaller";
    qDebug()<<bufferSplit;
}

else if(l==PLOT_COUNT+1)//if length of buffer is = no of plots then plot
{
    for(int i=0;i<l-1;++i)
    {
        ui->CustomPlot->graph(i)->addData(key,bufferSplit[i].toDouble());
        ui->CustomPlot->graph(i)->rescaleValueAxis();
        ui->CustomPlot->xAxis->setRange(key,8, Qt::AlignRight);
        ui->CustomPlot->yAxis->setRange(-10,10);
        ui->CustomPlot->replot();
        lastPointKey=key;
    }

    serialBuffer.clear();
}

else if(l>PLOT_COUNT+1)//if length of buffer is larger than no of plots
{
    qDebug()<<"Buffer Larger:"<<counter;
    qDebug()<<"Buffer Split:" <<bufferSplit;
    counter++;
    serialBuffer.clear();

}

}

*/

