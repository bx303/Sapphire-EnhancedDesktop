#ifndef SVAL_H
#define SVAL_H

#include "global.h"
#include "qdebug.h"
#include "qdialog.h"
#include "qobject.h"
#include "qsettings.h"
#include "qtreewidget.h"
#include "sfieldswidget.h"
#include "stylesettotal.h"
#include "ui_styleSetting.h"
#include"mainwindow.h"

template <class T>
struct Val: public QObject {

public:
    //值指针
    T* pval;
    //值名（代码变量名）
    QString name;
    //全名（fields+name)
    QString fullName;

    //域s（从顶级开始）
    QStringList fields;
    //显示名，向用户显示
    QString displayName;
    //用类似于QProperty的val与set方法，可使得在程序其他位置也可以调用set并触发信号
    T val()
    {
        return *pval;
    };

    //是否占用小空间(布局使用)
    bool issmall = false;

    virtual bool set(T newVal)
    {
        if(newVal == *pval) {
            return false;
        }
        *pval = newVal;
        if(!onLoading && activepmw != nullptr) {
            activepmw->endUpdate();
            activepmw->update();
        }
        return true;
    }

    Val<T>(const QStringList& fields, QString name, T* pval, const QString& displayName): pval(pval), fields(fields), name(name), displayName(displayName)
    {
        fullName = fields.join("/") + name;
    };


    //外部调用封装方法
    virtual void read(QSettings *styleIni)
    {
        if(styleIni->contains(name)) {
            readVal(styleIni);
            qDebug() << "Read" << name << ":" << val();
        } else {
            qDebug() << "No exist" << name << ",use default" << val();
        }
    }

    void write(QSettings *styleIni)
    {
        writeVal(styleIni);
        qDebug() << "Wrote" << name << ":" << val();
    }

    QWidget* valWidget = nullptr;
    bool operator<(const  Val<T>& another)const
    {
        if(fields == another.fields) {
            return name < another.name;
        } else {
            return fields < another.fields;
        }
    }


private:
    //从ini读值的主方法
    virtual void readVal(QSettings *styleIni) = 0;
    //写入ini的主方法
    virtual void writeVal(QSettings *styleIni) = 0;

};



template<class T>
struct LimitedVal: public Val<T> {
    T min;
    T max;
    LimitedVal<T>(const QStringList& fields, QString name, T* pval, const QString& displayName, T min, T max): Val<T>(fields, name, pval, displayName)
    {
        this->min = min;
        this->max = max;
    };

    //重载以实行范围检查
    bool set(T newVal)
    {
        return Val<T>::set(qBound(min, newVal, max));
    }
};

struct boolVal: public Val<bool> {
    Q_OBJECT
    using Val<bool>::Val;
    virtual void readVal(QSettings *styleIni)
    {
        set(styleIni->value(name).toBool());
    };
    virtual void writeVal(QSettings *styleIni)
    {
        styleIni->setValue(name, QString::number(val()));
    }


    //set函数以触发相应类型的信号
public slots:
    virtual bool set(bool newVal)
    {
        if(Val::set(newVal)) {
            emit valueChanged(newVal);
            return true;
        } else {
            return false;
        }
    }

public:
signals:
    void valueChanged(bool val);
};

struct intVal: public LimitedVal<int> {
    Q_OBJECT;
    using LimitedVal<int>::LimitedVal;
    virtual void readVal(QSettings *styleIni)
    {
        set(qBound( min, styleIni->value(name).toInt(), max));
    };
    virtual void writeVal(QSettings *styleIni)
    {
        styleIni->setValue(name, QString::number(val()));
    }


public slots:
    virtual bool set(int newVal)
    {
        if(LimitedVal::set(newVal)) {
            emit valueChanged(newVal);
            return true;
        } else {
            return false;
        }
    }

public:
signals:
    void valueChanged(int val);

};

struct doubleVal: public LimitedVal<double> {
    Q_OBJECT;
    using LimitedVal<double>::LimitedVal;
    virtual void readVal(QSettings *styleIni)
    {
        set(qBound( min, styleIni->value(name).toDouble(), max));
    };
    virtual void writeVal(QSettings *styleIni)
    {
        styleIni->setValue(name, QString::number(val()));
    }

public slots:
    virtual bool set(double newVal)
    {
        if(abs(val() - newVal) <= 0.01) {
            return false;
        }
        if(LimitedVal::set(newVal)) {
            qDebug() << name << newVal << val();
            emit valueChanged(newVal);
            return true;
        } else {
            return false;
        }
    }

public:
signals:
    void valueChanged(double val);
};

struct stringVal: public Val<QString> {
    Q_OBJECT;
    using Val<QString>::Val;
    virtual void readVal(QSettings *styleIni)
    {
        set(styleIni->value(name).toString());
    };
    virtual void writeVal(QSettings *styleIni)
    {
        styleIni->setValue(name, val());
    }
public:
public slots:
    virtual bool set(QString newVal)
    {
        if(Val::set(newVal)) {
            emit valueChanged(newVal);
            return true;
        } else {
            return false;
        }
    }

public:
signals:
    void valueChanged(QString val);
};

struct colorVal: public Val<QColor> {
    Q_OBJECT;
    using Val<QColor>::Val;
    virtual void readVal(QSettings *styleIni)
    {
        set(QColor(styleIni->value(name).toString().toUInt(NULL, 16)));
    };
    virtual void writeVal(QSettings *styleIni)
    {
        styleIni->setValue(name, QString::number(qRgb(val().red(), val().green(), val().blue()), 16));
    }
public:
public slots:
    virtual bool set(QColor newVal)
    {
        if(Val::set(newVal)) {
            emit valueChanged(newVal);
            return true;
        } else {
            return false;
        }
    }

public:
signals:
    void valueChanged(QColor val);
};



#endif // SVAL_H
