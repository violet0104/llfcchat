#include "global.h"

QString gate_url_prefix = "";

std::function<void(QWidget*)> repolish = [](QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input) {
    // 获取输入字符串长度
    int length = input.length();
    // 密钥限制在 0~254 范围
    uchar key = static_cast<uchar>(length % 255);
    QString result;
    result.reserve(length);

    // 逐字符异或加密/解密
    for (const QChar& ch : input) {
        ushort unicode = ch.unicode();
        result += QChar(unicode ^ key);
    }

    return result;
};
