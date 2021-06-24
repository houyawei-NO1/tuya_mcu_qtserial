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
      SerialTuya->setBaudRate(QSerialPort::Baud115200,QSerialPort::AllDirections);//设置波特率和读写方向
      SerialTuya->setDataBits(QSerialPort::Data8);		//数据位为8位
      SerialTuya->setFlowControl(QSerialPort::NoFlowControl);//无流控制
      SerialTuya->setParity(QSerialPort::NoParity);	//无校验位
      SerialTuya->setStopBits(QSerialPort::OneStop); //一位停止位

}
QByteArray SendData(QByteArray command,QString data,bool IsHex)//comand(0x**),data(**)
{
    QString temp;
    QByteArray dataHex,dataHex_len,dataFirst,dataFinal;
    QByteArray McuHead = QByteArray::fromHex("55aa03");
//    command=QByteArray::fromHex(data.toLatin1());
    qDebug()<<"command"<<command.toHex();
    qDebug()<<"McuHead"<<McuHead.toHex();
    if(IsHex == true)
    dataHex = QByteArray::fromHex(data.toLatin1());
    else
    dataHex = QByteArray::fromHex(data.toLatin1().toHex());
    qDebug()<<"data.toLatin1()"<<data.toLatin1().toHex();
//    if(dataHex.length()==1)
//    dataHex_len = QByteArray::fromHex("00") + QString(dataHex.length()).toLatin1();
//    else
//    dataHex_len = QString(dataHex.length()).toLatin1();
    dataHex_len = QByteArray::fromHex(QString("%1").arg(dataHex.length(), 4, 16, QLatin1Char('0')).toLatin1());
    qDebug()<<"datahex"<<dataHex<<"datahex_len"<<dataHex_len<<endl;
    dataFirst = McuHead + command + dataHex_len + dataHex;
    qDebug()<<"McuHead"<<McuHead.toHex()<<"command"<<command.toHex()<<"dataHex_len"<<dataHex_len.toHex()<<"dataFirst:"<<dataFirst.toHex()<<endl;
    temp = QString(dataFirst.toHex());
    qDebug()<<"temp:"<<temp<<endl;
    int byte_num = temp.length();
    static int sum = 0 ;
    for(int i = 0;i<byte_num;i=i+2)
    {
        QByteArray temp_add = QByteArray::fromHex(temp.mid(i,2).toLatin1());
        bool ok;
        int dec = temp_add.toHex().toInt(&ok,16);
        qDebug()<<"i"<<i<<dec<<endl;
        sum = sum + dec;
    }
    sum = sum%256;
    qDebug()<<"byte_num"<<byte_num<<"sum:"<<QByteArray::fromHex(QString("%1").arg(sum, 2, 16, QLatin1Char('0')).toLatin1())<<endl;
    temp = temp + QString("%1").arg(sum, 2, 16, QLatin1Char('0'));
//    qDebug()<<"temp:"<<temp<<endl;
    dataFinal = dataFirst + QByteArray::fromHex(QString("%1").arg(sum, 2, 16, QLatin1Char('0')).toLatin1());
    qDebug()<<"dataFinal:"<<dataFinal.toHex()<<endl;


}
void SerialMean(QString read)
{
    read = "55aa00000000";
//    read.remove(read.size()-2,2);
//    qDebug()<<read<<endl;
//    int byte_num = read.size()/2;
//    static int sum = 0 ;
//    for(int i = 0;i<byte_num;i=i+2)
//    {
//        QByteArray read1 = QByteArray::fromHex(read.mid(i,2).toLatin1());
//        bool ok;
//        int dec = read1.toHex().toInt(&ok,16);
//        qDebug()<<dec<<endl;
//        sum = sum + dec;
//    }
//    sum = sum%256;
//    qDebug()<<"sum:"<<sum<<endl;

//    read = read + QString(QByteArray::number(sum, 16));
//    qDebug()<<"read:"<<read<<endl;

    //tuya moudle send head(55aa)+version(00)+command(xx)+datalen(xx)+data(xx...)+datacheck(xx)
    //heartcheck comand(0x00)
    bool heartcheck = true;
    if(read.indexOf("55aa0000")==0)
    {
        if(heartcheck==true)//data(0x00)
        {

        QByteArray SendToTuya1 = SendData(QByteArray::fromHex("00"),"00",true);
        QByteArray SendToTuya2 = SendData(QByteArray::fromHex("00"),"01",true);
        SerialTuya->write(SendToTuya1);
        heartcheck=false;
        }
        else//data(0x01)
        SerialTuya->write(QByteArray::fromHex("55aa030000010104"));
    }
    //productcheck
    else if(read.indexOf("55aa0001")==0)
    {
        QString write;
        write="{\"p\":\"RN2FVAgXG6W****\",\"v\":\"1.0.0\",\"m\"}";
        QByteArray wr = write.toUtf8().toHex();
        SerialTuya->write(QByteArray::fromHex("55aa030000010104"));
        qDebug()<<"{"<<wr<<endl;
    }
    //operation mode
     else if(read.indexOf("55aa0003")==0)
    {
         SerialTuya->write(QByteArray::fromHex("55aa0303000005"));
    }
    //Data Point
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
    SerialMean("test");
    return a.exec();
}

