#include "ck_converter.h"
#include "ui_ck_converter.h"
#include <QtWidgets>
#include <QDebug>

CK_Converter::CK_Converter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CK_Converter)
{
    ui->setupUi(this);
}

CK_Converter::~CK_Converter()
{
    delete ui;
}

/*
 * source파일로부터 한줄씩 읽어들인 문단을 탭문자(\t)를 기준으로 title과 content로 나누고,
 * title 파일에 title의 내용을 써 넣고 다시 content 파일에 content의 내용을 써 넣는다.
 * 다시 title 파일에 content 파일의 파일포인터와 content 길이를 첨부하여
 * 인덱스파일을 만든다.
 */
void CK_Converter::on_btnLoadAndSplitTabFile_clicked()
{
    QString buffer;
    QString bufTitle;
    QString bufContent;
    QStringMatcher strMatcher;

    QFile source(this);
    source.setFileName("../CyberK_Converter/a.txt");

    QFile targetTitle(this);
    QFile targetContent(this);
    targetTitle.setFileName("../CyberK_Converter/title.txt");
    targetContent.setFileName("../CyberK_Converter/content.txt");

    //Read from file
    if(!source.open(QFile::ReadOnly|QFile::Text))
    {
        //파일 열기에 실패하면 표시하는 메시지
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(tr("Warning!!"));
        msgBox->setText(tr("The file can't be opened. Please check if it is in the right folder"));
        msgBox->show();
        return;
    }else{
        QTextStream inFromSource;
        strMatcher.setPattern("\t");
        qint8 splitPos; //라인단위로 읽어올 것이므로 splitPos는 qint8로도 충분할 듯하다.

        //입력스트림과 원본 파일을 연결
        inFromSource.setDevice(&source);
        inFromSource.setCodec("UTF-8");

        QDataStream outToTitle; //title파일은 숫자를 포함하는 이진파일이므로 QDataStream사용
        outToTitle.setDevice(&targetTitle);
        //QDataStream의 경우 딸려오는 정보들이 버전별로 다른가 보다. 버전을 맞춰서 읽고 써야 한다.
        outToTitle.setVersion(QDataStream::Qt_5_5);
        outToTitle.device()->open(QFile::WriteOnly);

        QTextStream outToContent;
        outToContent.setDevice(&targetContent);
        outToContent.device()->open(QFile::WriteOnly|QFile::Text);
        outToContent.setCodec("UTF-8");//Qt는 내부적으로 유니코드에 utf-16을 쓰므로 utf-8은 명시적으로 코덱을 써줘야 한다.

        //라인단위로 읽어들이므로 전체 파일에서의 위치를 기억할 변수가 필요하다.
        qint64 posOld = 0;

        while(!inFromSource.atEnd())
        {
            buffer = inFromSource.readLine();
            //buffer스트링의 첫번째에서 시작해서 strMatcher에 지정된 패턴(여기서는 "\t")을 찾는다.
            splitPos = strMatcher.indexIn(buffer, 0);
            //buffer스트링의 첫번째에서 시작해서 처음 패턴이 찾아진 곳까지가 title이다.
            bufTitle = buffer.mid(0,splitPos);
            //buffer스트링의 총길이에서 \t가 찾아진 위치를 빼면 content의 길이가 나온다.
            //buffer스트링에서 \t가 찾아진 위치로부터 시작해서 content의 길이만큼을 잘라낸후 \t를 제거하면 content이다.
            bufContent = buffer.mid(splitPos,buffer.length()-splitPos).trimmed();//처음에 붙은 탭문자제거

            //plainTextEdit에 임시로 출력해 본다.
            ui->plainTextEdit->appendPlainText(bufTitle+" "+bufContent);

            //인덱스파일의 스트림(outToTitle)에 타이틀을 써 넣는다.
            outToTitle << bufTitle;
            //본문파일의 스트림(outToContent)에 본문을 써 넣는다.
            outToContent << bufContent;
            //인덱스파일의 스트림(outToTitle)에 본문파일의 스트림(outToContent)의 파일포인터, content의 글자수를 써 넣는다.
            outToTitle << posOld << bufContent.length();
            //파일포인터를 content뒤쪽으로 옮긴다. 단, 글자수가 아닌 바이트수로 계산해야 한다.
            posOld += bufContent.toUtf8().size();
        }
        outToContent.flush();
        targetTitle.flush();
        targetTitle.close();
        targetContent.flush();
        targetContent.close();
        source.flush();
        source.close();
    }
}

/*title파일을 읽어 map자료구조에 넣고 list에 title만을 표시한다.*/
void CK_Converter::on_btnReload_clicked()
{
    QFile sourceTitle;
    sourceTitle.setFileName("../CyberK_Converter/title.txt");
    QFile sourceContent;
    sourceContent.setFileName("../CyberK_Converter/content.txt");

    QDataStream inFromTitle;
    inFromTitle.setDevice(&sourceTitle);
    inFromTitle.setVersion(QDataStream::Qt_5_4);

    inFromTitle.device()->open(QFile::ReadOnly);
    
    QString Title;
    _position offset;//title파일에 기록된 content의 포인터와 길이를 받아오기 위한 구조체

    while(!inFromTitle.atEnd())
    {
        inFromTitle >> Title >> offset.contentBegin >> offset.contentLength;
        map.insert(Title,offset); //string과 구조체를 저장하는 자료구조(map)
    }

    sourceTitle.close();

    //map을 순환하기 위한 반복자
    QMapIterator<QString, _position> itor(map);
    while (itor.hasNext())
    {
        itor.next();
        ui->listWidget->addItem(itor.key().toUtf8());
    }
}

//list에서 title이 선택되면 map에서 키에 해당하는
void CK_Converter::on_listWidget_itemSelectionChanged()
{
    QFile sourceContent;
    sourceContent.setFileName("../CyberK_Converter/content.txt");

    QTextStream inFromContent;
    inFromContent.setDevice(&sourceContent);
    inFromContent.setCodec("UTF-8"); //Qt는 내부적으로 유니코드에 utf-16을 사용하므로 utf-8은 이렇게 지정해줘야 한다.

    //파일오픈에 실해했을 경우 출력 메시지
    if(!inFromContent.device()->open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("No Content file in the directory");
        msgBox->show();
    }

    //키(현재 선택된 title)에 해당하는 value(시작위치,길이)를 찾아서 offset 구조체에 저장
    _position offset;
    offset = map.value(ui->listWidget->currentItem()->text());

    QString Content;
    //시작위치(읽을 위치)로 파일 포인터를 이동
    inFromContent.seek(offset.contentBegin);
    //현재의 파일 포인터에서 지정된 길이만큼 읽음
    Content = inFromContent.read(qint64(offset.contentLength));
    //읽은 내용을 plainTextEdit에 표시
    ui->plainTextEdit_2->setPlainText(Content);

    inFromContent.flush();
    sourceContent.close();
}
