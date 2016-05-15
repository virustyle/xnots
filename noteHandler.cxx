#include "noteHandler.h"
extern "C" {
    #include <mkdio.h>
}

noteHandler::noteHandler(QDir *notesDir)
{
    dir = notesDir;
    windows = new QHash<QString, QTextEdit*>();
    scanDir();
    timer.start((1000)*10,this);
}

noteHandler::~noteHandler()
{
    delete windows;
}

void noteHandler::timerEvent(QTimerEvent *event)
{
    scanDir();
}

QString *noteHandler::readNote(const QString filename)
{
    QFile *file = new QFile(filename);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text ))
    {
        qWarning("ERROR: File %s unreadable", qPrintable(filename));
        return NULL;
    }

    QTextStream in(file);
    bool handlingOptions = true;
    QString *noteText = new QString();
    while ( !in.atEnd() ) {
        QString line = in.readLine();
        if ( handlingOptions && line.startsWith("*") )
        {
            // TODO: Write handler for in-note options
        }
        else if ( handlingOptions && !line.isEmpty() )
        {
            handlingOptions = false;
        }
        if ( !handlingOptions )
        {
            noteText->append(line);
            noteText->append("\n");
        }
    }

    char *htmlBuf = 0;
    MMIOT *doc = mkd_string(qPrintable(*noteText),noteText->length(),0);
    mkd_compile(doc,0);
    mkd_document(doc,&htmlBuf);
    delete noteText;
    delete file;
    return new QString(htmlBuf);
}

void noteHandler::scanDir()
{
    dir->refresh();
    QStringList notes = dir->entryList(QDir::Files);
    
    QHash<QString, QTextEdit *>::iterator iter = windows->begin();
    while ( iter != windows->end() )
    {
      if ( !notes.contains(iter.key()) )
      {
	iter.value()->close();
	iter = windows->erase(iter);
      }
      else
      {
	iter.value()->setHtml(*readNote(iter.key()));
	iter++;
      }
    }

    for ( int i=0; i < notes.size(); i++ )
    {
        if ( !windows->contains(notes.at(i)) )
        {
	    QTextEdit *textEdit = new QTextEdit();
        QString *noteText = readNote(notes.at(i));
	    if ( noteText == NULL )
	    {
		continue;
	    }
	    textEdit->setHtml(*noteText);
	    textEdit->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowStaysOnBottomHint | Qt::WindowCloseButtonHint | Qt::Tool);
	    textEdit->show();
        windows->insert(notes.at(i), textEdit);
        delete noteText;
	}
    }
}
