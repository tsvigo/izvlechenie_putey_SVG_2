#include "dialog.h"

#include <QApplication>
#//----------------------------------------------------------------------------------------------------------------

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QDir>
#include <QDebug>

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
#//-------------------------------------------------------------------------------------------------------------
int main(
    int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.show();
#//-------------------------------------------------------------------------------------------------------------------------
    QString currentPath = QDir::currentPath();
    qDebug() << "\n" ;
    qDebug() << "Текущий рабочий каталог:" << currentPath;
    qDebug() << "\n" ;

    // Новый путь, в который нужно сменить рабочий каталог
    QDir appDir(QCoreApplication::applicationDirPath());
    QString newDirPath = appDir.absolutePath();
        //"/путь/к/новому/каталогу";

    // Пытаемся установить новый рабочий каталог
    if (QDir::setCurrent(newDirPath)) {
        qDebug() << "Текущий рабочий каталог успешно изменен на:" << QDir::currentPath()<<"\n";
    } else {
        qDebug() << "Не удалось изменить рабочий каталог на:" << newDirPath<<"\n";
    }
#//----------------------------------------------------------------------------------------------------------------------

    // 1. Открытие диалога выбора SVG-файла
    QString fileName = QFileDialog::getOpenFileName(nullptr,
                                                    "Выберите SVG файл",
                                                    "",
                                                    "SVG файлы (*.svg)");
    if (fileName.isEmpty()) {
        qDebug() << "Файл не выбран.";
        return 0;
    }
#//--------------------------------------------------------------------------------------------------- \ \
    // Открываем файл svg_pathname.txt для записи
    QFile outputFile("svg_pathname.txt");
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл для записи.";
        return 1;
    }

    // Записываем путь в файл
    QTextStream out(&outputFile);
    out << fileName;
    outputFile.close();

    qDebug() << "Путь к файлу записан в svg_pathname.txt"<< "\n";
 //  return 0;
#//---------------------------------------------------------------------------------------------------------------
    // 2. Чтение содержимого файла
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл:" << fileName;
        return 1;
    }
    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // 3. Извлечение всех элементов <path ...>
    // Учтём как самозакрывающийся тег, так и парный вариант
    QRegularExpression pathRegex("<path\\b([^>]*)(?:\\/?>|>.*?<\\/path>)");
    QRegularExpressionMatchIterator it = pathRegex.globalMatch(fileContent);

    int pathIndex = 1;
    // Заголовок и футер для минимального SVG-файла
    QString svgHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    QString svgFooter = "\n</svg>\n";

    // 4. Обработка каждого найденного пути
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString pathElement = match.captured(0);
        qDebug() << "Найден path:" << pathElement;

        // Формирование имени папки (например, "1", "2", …)
        QString folderName = QString::number(pathIndex);
        QDir dir;
        // Создание папки. Если она уже существует, mkpath ничего не изменит.
        if (!dir.mkpath(folderName)) {
            qDebug() << "Не удалось создать папку:" << folderName;
            pathIndex++;
            continue;
        }


#//---------------------------------------------------------------------------------------------------------



#//----------------------------------------------------------------------------------------------
        // Формирование полного имени выходного файла, например: "1/puty1.svg"
        QString outFileName = QString(
                                      "%1/puty%2.svg").arg(folderName).arg(pathIndex);
        QFile outFile(outFileName);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Ошибка открытия файла для записи:" << outFileName;
            pathIndex++;
            continue;
        }
        QTextStream out(&outFile);
        // Запись минимального SVG-файла с заголовком, найденным путем и футером
        out << svgHeader << pathElement << svgFooter;
        outFile.close();
qDebug() << "\n" ;
        qDebug() << "Сохранён файл:" << outFileName;
        qDebug() << "\n" ;
        pathIndex++;
    }

    if (pathIndex == 1) {
        qDebug() << "Элементы <path> не найдены в файле.";
    }
#//--------------------------------------------------------------------------------------------------------------
    return a.exec();
}
