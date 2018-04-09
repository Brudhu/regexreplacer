#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDirIterator>
#include <QRegularExpression>
#include <QDebug>

class MyOutput
{
public:
    MyOutput(int verbosityThresh)
    {
        this->verbosityThresh = verbosityThresh;
    }

    void printOutput(QtMsgType type, int verbosityLevel, const QString &msg)
    {
        QByteArray localMsg = msg.toLocal8Bit();
        if(verbosityLevel <= this->verbosityThresh)
            switch (type) {
            case QtInfoMsg:
                fprintf(stderr, "[info] %s\n", localMsg.constData());
                break;
            case QtDebugMsg:
                fprintf(stderr, "[debug] %s\n", localMsg.constData());
                break;
            case QtWarningMsg:
                fprintf(stderr, "[warning] %s\n", localMsg.constData());
                break;
            case QtCriticalMsg:
                fprintf(stderr, "[critical] %s\n", localMsg.constData());
                break;
            case QtFatalMsg:
                fprintf(stderr, "[fatal] %s\n", localMsg.constData());
                abort();
            }
    }

private:
    int verbosityThresh;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("RegexReplacer");
    QCoreApplication::setApplicationVersion("1.0");

    qSetMessagePattern("[%{type}] %{message}");

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt application to replace strings in files using regular expressions. "
                                     "It can be used with a single file or a directory - in this case, it will "
                                     "replace strings in every file inside the directory.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("path", QCoreApplication::translate("main", "Path to file or directory to replace strings."));
    parser.addPositionalArgument("regexSource", QCoreApplication::translate("main", "Regex to capture strings from file(s)."));
    parser.addPositionalArgument("regexDest", QCoreApplication::translate("main", "Regex with new string to replace in file(s)."));

    QCommandLineOption v("verbosity",
            QCoreApplication::translate("main", "Set application verbosity level. From 0 (silent) to 4 (very verbose). Default: 3"),
            QCoreApplication::translate("main", "directory"),
            "3");
    parser.addOption(v);
    parser.process(a);

    int verbosity = parser.value(v).toInt();
    const QStringList args = parser.positionalArguments();

    if(args.length() != 3)
    {
        parser.showHelp();
    }

    MyOutput output(verbosity);
    output.printOutput(QtInfoMsg, 1, QString("Starting regexreplacer..."));

    QFileInfo path(args.at(0));
    QRegularExpression regexSource(args.at(1));
    if(!regexSource.isValid())
    {
        output.printOutput(QtInfoMsg, 1, QString("Invalid regex: \"%1\" - aborting...").arg(regexSource.pattern()));
        return -1;
    }

    QString regexDest(args.at(2));

    if(path.isDir()) // If it's a directory, enter and do the regex replacing in all the files.
    {
        output.printOutput(QtInfoMsg, 3, QString("Argument \"Path\" is a directory. Accessing it (%1)...").arg(path.path()));
        QDirIterator dirIt(path.path(), QDirIterator::NoIteratorFlags);

        while(dirIt.hasNext())
        {
            QString filePathStr = dirIt.next();
            QFileInfo filePath(filePathStr);
            if(!filePath.isFile())
            {
                continue;
            }

            QFile f(filePathStr);
            if(!f.open(QIODevice::ReadWrite))
            {
                output.printOutput(QtInfoMsg, 2, QString("%1 - Unable to open.").arg(f.fileName()));
                continue;
            }

            QString content = QString(f.readAll());
            int matches = content.count(regexSource);
            content.replace(regexSource, regexDest);
            output.printOutput(QtInfoMsg, 3, QString("%1 - Replacing strings... Found %2 match(es).").arg(f.fileName(), QString::number(matches)));

            f.resize(0);
            f.write(content.toUtf8());
        }
    }
    else if(path.isFile())
    {
        QFile f(path.filePath());

        output.printOutput(QtInfoMsg, 3, "Argument \"Path\" is a file.");
        if(!f.open(QIODevice::ReadWrite))
        {
            output.printOutput(QtInfoMsg, 2, QString("%1 - Unable to open.").arg(f.fileName()));
        }
        else
        {
            QString content = QString(f.readAll());
            int matches = content.count(regexSource);
            content.replace(regexSource, regexDest);
            output.printOutput(QtInfoMsg, 3, QString("%1 - Replacing strings... Found %2 match(es).").arg(f.fileName(), QString::number(matches)));

            f.resize(0);
            f.write(content.toUtf8());
        }
    }

    output.printOutput(QtInfoMsg, 1, QString("Done!"));

    return 0;
}
