#include "httpmgr.h"

HttpMgr::HttpMgr()
{

}

HttpMgr::~HttpMgr()
{
    //连接http请求和完成信号，信号槽机制保证队列消费
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    auto self = shared_from_this();
    /* 当通过 QNetworkAccessManager 发送网络请求后，会返回一个 QNetworkReply 对象
        该对象包含了服务器的响应数据以及与响应相关的其他信息，当网络请求完成时，reply会发出一个finished信号
    */
    QNetworkReply* reply = _manager.post(request, data);
    /* 由于网络请求是异步的：收到信号前窗口可能被关掉、被销毁，导致this变成悬空指针，
        这时lambda再执行会导致程序直接崩溃，因此要用share_from_this()智能指针
    */
    QObject::connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod](){
        // 处理错误情况
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            // 发送信号通知完成
            emit self->sig_http_finish(req_id, "", ErrorCodes::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }

        // 无错误
        QString res = reply->readAll();
        // 发送信号通知完成
        emit self->sig_http_finish(req_id, res, ErrorCodes::SUCCESS, mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if (mod == Modules::REGISTERMOD) {
        // 发送信号通知指定模块http的响应结束了
        emit sig_reg_mod_finish(id, res, err);
    }
}

