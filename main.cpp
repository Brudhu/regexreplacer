#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDirIterator>
#include <QRegularExpression>
#include <QDebug>

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

    QCommandLineOption v("verbose", QCoreApplication::translate("main", "Run application in verbose mode."));
    parser.addOption(v);

    parser.process(a);

    bool verbose = parser.isSet(v);
    const QStringList args = parser.positionalArguments();

    if(args.length() != 3)
    {
        parser.showHelp();
    }

    QFileInfo path(args.at(0));
    QRegularExpression regexSource(args.at(1));
    if(!regexSource.isValid())
    {
        qInfo() << "Invalid regex:" << regexSource.pattern() << "- aborting...";
        return -1;
    }

    QString regexDest(args.at(2));

    if(path.isDir()) // If it's a directory, enter and do the regex replacing in all the files.
    {
        qInfo() << "Argument \"Path\" is a directory. Accessing it...";
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
                qInfo() << f.fileName() << "- Unable to open.";
                continue;
            }

            QString content = QString(f.readAll());
            int matches = content.count(regexSource);
            content.replace(regexSource, regexDest);
            qInfo() << f.fileName() << "- Replacing strings... Found" << matches << "matches.";

            f.resize(0);
            f.write(content.toUtf8());
        }
    }
    else if(path.isFile())
    {
        QFile f(path.filePath());

        qInfo() << "Argument \"Path\" is a file.";
        if(!f.open(QIODevice::ReadWrite))
        {
            qInfo() << f.fileName() << "- Unable to open.";
        }
        else
        {
            QString content = QString(f.readAll());
            int matches = content.count(regexSource);
            content.replace(regexSource, regexDest);
            qInfo() << f.fileName() << "- Replacing strings... Found" << matches << "matches.";

            f.resize(0);
            f.write(content.toUtf8());
        }
    }

    qInfo() << "Done!";

    return 0;
}
