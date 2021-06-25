#include <QCoreApplication>
#include <QSerialPort>
#include <QDebug>
#include <QLatin1Char>
static QSerialPort *SerialTuya;

void StartSerialTuya()
{
    qDebug()<<"##########################StartSerialTuya"<<endl;
    SerialTuya = new QSerialPort();
    if(SerialTuya->isOpen())
    {
        SerialTuya->clear();
        SerialTuya->close();
    }
    SerialTuya->setPortName("/dev/ttyAMA0");
//    SerialTuya->setPortName("/dev/ttyUSB0");
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
//    command=QByteArray::fromHex(data.toLatin1());
//    qDebug()<<"command"<<command.toHex();
//    qDebug()<<"McuHead"<<McuHead.toHex();
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
//    qDebug()<<"datahex"<<dataHex<<"datahex_len"<<dataHex_len<<endl;
    dataFirst = McuHead + command + dataHex_len + dataHex;
    qDebug()<<"McuHead"<<McuHead.toHex()<<"command"<<command.toHex()<<"dataHex_len"<<dataHex_len.toHex()<<"dataFirst:"<<dataFirst.toHex()<<endl;
    temp = QString(dataFirst.toHex());
//    qDebug()<<"temp:"<<temp<<endl;
    int byte_num = temp.length();
    static int sum = 0 ;
    for(int i = 0;i<byte_num;i=i+2)
    {
        QByteArray temp_add = QByteArray::fromHex(temp.mid(i,2).toLatin1());
        bool ok;
        int dec = temp_add.toHex().toInt(&ok,16);
        sum = sum + dec;
//        qDebug()<<"i"<<i<<dec<<"SUM"<<sum<<endl;
    }
    sum = sum%256;
//    qDebug()<<"byte_num"<<byte_num<<"sum:"<<QByteArray::fromHex(QString("%1").arg(sum, 2, 16, QLatin1Char('0')).toLatin1())<<endl;
    temp = temp + QString("%1").arg(sum, 2, 16, QLatin1Char('0'));
//    qDebug()<<"temp:"<<temp<<endl;
    dataFinal = dataFirst + QByteArray::fromHex(QString("%1").arg(sum, 2, 16, QLatin1Char('0')).toLatin1());
    qDebug()<<"dataFinal:"<<dataFinal.toHex()<<endl;
    sum = 0 ;
return dataFinal.toHex();
}
void SerialMean(QString read)
{

    //tuya moudle send head(55aa)+version(00)+command(xx)+datalen(xx)+data(xx...)+datacheck(xx)
//    read = "55aa00000000ff";//tuya moudle heartcheck
     read = "55aa0001000000";//tuya moudle productcheck
    //heartcheck comand(0x00)
    bool static heartcheck = true;
    if(read.indexOf("55aa0000")==0)
    {
        if(heartcheck==true)//data(0x00)
        {
        QByteArray SendToTuya = SendData(QByteArray::fromHex("00"),"00",true);
//        SerialTuya->write(SendToTuya);
         qDebug()<<"SendToTuya1"<<SendToTuya;
        heartcheck=false;
        }
        else//data(0x01)
        {
        QByteArray SendToTuya = SendData(QByteArray::fromHex("00"),"01",true);
//        SerialTuya->write(SendToTuya);
         qDebug()<<"SendToTuya2"<<SendToTuya;
        }
    }
    //productcheck comand(0x01)
    else if(read.indexOf("55aa0001")==0)
    {
        QString ProductMessage;
        ProductMessage="{\"p\":\"RN2FVAgXG6WfAktU\",\"v\":\"1.0.0\",\"m\":0}";
        QByteArray ProductMessageBytes = ProductMessage.toUtf8().toHex();
//        SerialTuya->write(QByteArray::fromHex("55aa030000010104"));
        qDebug()<<"ProductMessageBytes:"<<ProductMessageBytes<<endl;
        QByteArray SendToTuya = SendData(QByteArray::fromHex("01"),ProductMessageBytes,true);
    }
    //operation mode
     else if(read.indexOf("55aa0003")==0)
    {
         SerialTuya->write(QByteArray::fromHex("55aa0303000005"));
    }
    //Data Point
 return;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    StartSerialTuya();
//    QString str = "{“p”:”RN2FVAgXG6W****”,“v”:”1.0.0”,”m”}";
//    qDebug()<<str<<endl;
//    QByteArray ascii = str.toUtf8().toHex();
//    qDebug()<<ascii<<endl;
//   QByteArray SendToTuya = SendData(QByteArray::fromHex("00"),"00",true);
    SerialMean("test1");
    SerialMean("test2");
    return a.exec();
}

