#include <QCoreApplication>
#include <QSerialPort>
#include <QDebug>
#include <QLatin1Char>
static QSerialPort *SerialTuya;

void StartSerialTuya()
{
    qDebug()<<"StartSerialTuya"<<endl;
    SerialTuya = new QSerialPort();
    if(SerialTuya->isOpen())
    {
        SerialTuya->clear();
        SerialTuya->close();
    }
//    SerialTuya->setPortName("/dev/ttyAMA0");
    SerialTuya->setPortName("/dev/ttyUSB0");
    if(!SerialTuya->open(QIODevice::ReadWrite))
    {
        qDebug()<<SerialTuya<<"打开失败!";
        return;
    }
      SerialTuya->setBaudRate(QSerialPort::Baud115200,QSerialPort::AllDirections);
      SerialTuya->setDataBits(QSerialPort::Data8);
      SerialTuya->setFlowControl(QSerialPort::NoFlowControl);
      SerialTuya->setParity(QSerialPort::NoParity);
      SerialTuya->setStopBits(QSerialPort::OneStop);

}
QByteArray SendData(QByteArray command,QString data,bool IsHex)//comand(0x**),data(**)
{
    QString temp;
    QByteArray dataHex,dataHex_len,dataFirst,dataFinal;
    QByteArray McuHead = QByteArray::fromHex("55aa03");
    if(IsHex == true)
    dataHex = QByteArray::fromHex(data.toLatin1());
    else
    dataHex = QByteArray::fromHex(data.toLatin1().toHex());
//    qDebug()<<"data.toLatin1()"<<data.toLatin1().toHex();
//    if(dataHex.length()==1)
//    dataHex_len = QByteArray::fromHex("00") + QString(dataHex.length()).toLatin1();
//    else
//    dataHex_len = QString(dataHex.length()).toLatin1();
    dataHex_len = QByteArray::fromHex(QString("%1").arg(dataHex.length(), 4, 16, QLatin1Char('0')).toLatin1());
    dataFirst = McuHead + command + dataHex_len + dataHex;
    qDebug()<<"McuHead"<<McuHead.toHex()<<"command"<<command.toHex()<<"data"<<dataHex.toHex()<<"dataHex_len"<<dataHex_len.toHex()<<"dataFirst:"<<dataFirst.toHex();
    temp = QString(dataFirst.toHex());
    int byte_num = temp.length();
    static int sum = 0 ;
    for(int i = 0;i<byte_num;i=i+2)
    {
        QByteArray temp_add = QByteArray::fromHex(temp.mid(i,2).toLatin1());
        bool ok;
        int dec = temp_add.toHex().toInt(&ok,16);
        sum = sum + dec;
    }
    sum = sum%256;
    temp = temp + QString("%1").arg(sum, 2, 16, QLatin1Char('0'));
    dataFinal = dataFirst + QByteArray::fromHex(QString("%1").arg(sum, 2, 16, QLatin1Char('0')).toLatin1());
    sum = 0 ;
return dataFinal;
}
void SerialMean(QString read)
{

    //tuya moudle send head(55aa)+version(00)+command(xx)+datalen(xx)+data(xx...)+datacheck(xx)

    //heartcheck comand(0x00)
    bool static heartcheck = true;
    if(read.indexOf("55aa0000")==0)
    {
        if(heartcheck==true)//data(0x00)
        {
        QByteArray SendToTuya = SendData(QByteArray::fromHex("00"),"00",true);
        SerialTuya->write(SendToTuya);
        qDebug()<<"SendToTuya_heartcheck_1"<<SendToTuya.toHex()<<endl;
        heartcheck=false;
        }
        else//data(0x01)
        {
        QByteArray SendToTuya = SendData(QByteArray::fromHex("00"),"",true);
        SerialTuya->write(SendToTuya);
        qDebug()<<"SendToTuya_heartcheck_2"<<SendToTuya.toHex()<<endl;
        }
    }
    //productcheck comand(0x01)
    else if(read.indexOf("55aa0001")==0)
    {
        QString ProductMessage;
        ProductMessage="{\"p\":\"brku1qm1civanw1y\",\"v\":\"1.0.0\",\"m\":0}";
        QByteArray ProductMessageBytes = ProductMessage.toUtf8().toHex();
        qDebug()<<"ProductMessageBytes:"<<ProductMessageBytes;
        QByteArray SendToTuya = SendData(QByteArray::fromHex("01"),ProductMessageBytes,true);
        SerialTuya->write(SendToTuya);
        qDebug()<<"SendToTuya_productcheck"<<SendToTuya.toHex()<<endl;
    }
    //operation mode comand(0x02)
     else if(read.indexOf("55aa0002")==0)
    {
        //only moudle
//         QByteArray SendToTuya = SendData(QByteArray::fromHex("02"),"0c0d",true);
        //moudle and mcu
         QByteArray SendToTuya = SendData(QByteArray::fromHex("02"),"",true);
         SerialTuya->write(SendToTuya);
         qDebug()<<"SendToTuya_operation mode"<<SendToTuya.toHex()<<endl;
    }

    //wifi status comand(0x03)
    else if(read.indexOf("55aa0003")==0)
    {
        QByteArray SendToTuya = SendData(QByteArray::fromHex("03"),"",true);
//         SerialTuya->write(QByteArray::fromHex("55aa0303000005"));
         qDebug()<<"wifi status"<<SendToTuya;
    }

     //Data Point up
    else if(read.indexOf("55aa0008")==0)
    {
//        datapoint  dpid05 type02 len0004 value0000001e
        QByteArray SendToTuya = SendData(QByteArray::fromHex("07"),"050200040000001e",true);
//        datapointarray[0]  dpid109 6d type01 len0001 value01
//        datapointarray[1]  dpid102 66 type03 len000c value323031383034313231353037
//         SerialTuya->write(QByteArray::fromHex("55aa0303000005"));
         qDebug()<<"Data Point up"<<SendToTuya;

         QByteArray SendToTuyaarray = SendData(QByteArray::fromHex("07"),"6d010001016603000c323031383034313231353037",true);

    }
 return;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //tuya moudle heartcheck
    QString heartcheck = "55aa00000000ff";
    //tuya moudle productcheck
    QString productcheck = "55aa0001000000";
    //tuya moudle operation modecheck
    QString modecheck = "55aa0002000001";
    SerialMean("55aa000300010003");//tuya moudle wifi status
    SerialMean("55aa0008000007");
    static QString readData;
    StartSerialTuya();
    QObject::connect(SerialTuya,&QSerialPort::readyRead,[=]{
        QByteArray receiveArray = SerialTuya->readAll();
        readData.append(QString(receiveArray.toHex()));
        qDebug()<<"receiveArray"<<readData;

        if(readData.indexOf(heartcheck)>-1)
        {
            readData.remove(0,readData.indexOf(heartcheck));
            readData.remove(0,heartcheck.length());
            SerialMean(heartcheck);
        }
        else if(readData.indexOf(productcheck)>-1)
        {
            readData.remove(0,readData.indexOf(productcheck));
            readData.remove(0,productcheck.length());
            SerialMean(productcheck);
        }
        else if(readData.indexOf(modecheck)>-1)
        {
            readData.remove(0,readData.indexOf(modecheck));
            readData.remove(0,modecheck.length());
            SerialMean(modecheck);
        }

    });


    return a.exec();
}

