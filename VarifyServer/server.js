var grpc = require('@grpc/grpc-js');
var uuidv4 = require('uuid').v4;
var emailModule = require('./email');
var const_module = require('./const');
var message_proto = require('./proto');

async function GetVarifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try{
        uniqueId = uuidv4();
        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: '3170877204@qq.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };
        
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res is ", send_res)
        
        callback(null, { email:  call.request.email,
            error:const_module.Errors.Success
        }); 
        
    }catch(error){
        console.log("catch error is ", error)
        
        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }
}

function main() {
    var server = new grpc.Server()
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')        
    })
}

main()
