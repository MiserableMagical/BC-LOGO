#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QStringList>
#include <QTextStream>

#include <cstdio>

namespace {

const char *kUsage =
    "BC LOGO headless runner\n"
    "  BC_LOGO.exe --script <file.lgo> [--out <report.txt>] [-platform offscreen]\n"
    "Without --script the GUI starts normally.\n";

QString takeValue(const QStringList &args, int &i)
{
    return (i + 1 < args.size()) ? args[++i] : QString();
}

bool writeReport(const QString &path, const QString &text)
{
    QFile out(path);
    if (!out.open(QIODevice::WriteOnly))
        return false;
    out.write(text.toUtf8());
    return true;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString scriptPath;
    QString outPath;

    const QStringList args = QCoreApplication::arguments();
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == "--script")
            scriptPath = takeValue(args, i);
        else if (args[i] == "--out")
            outPath = takeValue(args, i);
        else if (args[i] == "--help" || args[i] == "-h") {
            QTextStream(stdout) << kUsage;
            return 0;
        }
    }

    MainWindow w;
    if (scriptPath.isEmpty()) {
        w.show();
        return a.exec();
    }

    QFile in(scriptPath);
    if (!in.open(QIODevice::ReadOnly)) {
        const QString msg = QString("error: cannot open script: %1\n").arg(scriptPath);
        if (!outPath.isEmpty())
            writeReport(outPath, msg);
        else
            QTextStream(stderr) << msg;
        return 2;
    }
    const QString text = QString::fromUtf8(in.readAll());
    in.close();

    /* 与交互式输入保持同一路径：逐行喂给 bufferExec()，
       这样 TO…END 的跨行定义行为和用户手敲完全一致。 */
    w.clearTranscript();
    const QStringList lines = text.split('\n');
    for (const QString &raw : lines) {
        QString line = raw;
        if (line.endsWith('\r'))
            line.chop(1);
        w.cmd_buf = line;
        w.bufferExec();
    }

    QString report = "--- state ---\n" + w.stateSnapshot()
                   + "--- transcript ---\n" + w.transcript();
    if (!report.endsWith('\n'))
        report += '\n';

    if (!outPath.isEmpty())
        writeReport(outPath, report);
    else
        QTextStream(stdout) << report;
    return 0;
}
