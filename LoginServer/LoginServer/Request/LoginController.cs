﻿using System;
using System.Threading.Tasks;
using System.Web.Http;

namespace LoginServer.Request
{
	public class LoginController : ApiController
	{
        [Route("Request/Login")]
        [HttpPost]
        public async Task<LoginRes> LoginRequest(LoginReq reqPacket)
        {
            var resPacket = new LoginRes();

            // 유저 패스워드를 암호화 한다.
            var encryptedPassword = Util.Encrypter.EncryptString(reqPacket.UserPw);

            Console.WriteLine($"Id({reqPacket.UserId}), Pw({encryptedPassword}), Send Login Request");

            // DB Server에 유저가 가입되어 있는지를 조사한다.
            var userValidationReq = new DBServer.UserValidationReq();
            userValidationReq.UserId = reqPacket.UserId;
            userValidationReq.EncryptedUserPw = encryptedPassword;

            var userValidationRes = await Util.HttpMessenger.RequestHttp<DBServer.UserValidationReq, DBServer.UserValidationRes>(
                "http://localhost:20000/", "DB/UserValidation", userValidationReq);

            // 가입되어 있지 않다면 에러 반환.
            if (userValidationRes.Result != (short)ErrorCode.None)
            {
                resPacket.Result = userValidationRes.Result;
                resPacket.Token = -1;
                return resPacket;
            }

            // 가입되어 있다면 토큰을 생성한다.
            resPacket.Token = TokenGenerator.GetInstance().CreateToken();

            // DB Server에 토큰을 등록한다.
            var tokenAuthReq = new DBServer.TokenAuthReq();
            tokenAuthReq.UserId = reqPacket.UserId;
            tokenAuthReq.Token = resPacket.Token;

            var tokenAuthRes = await Util.HttpMessenger.RequestHttp<DBServer.TokenAuthReq, DBServer.TokenAuthRes>(
                "http://localhost:20000/", "DB/RegistToken", tokenAuthReq);

            // 토큰 등록이 실패했다면 에러 반환.
            if (tokenAuthRes.Result != (short)ErrorCode.None)
            {
                resPacket.Result = userValidationRes.Result;
                resPacket.Token = -1;
                return resPacket;
            }

            // 모든 절차가 완료되었다면 정상값 반환.
            resPacket.Result = (short)ErrorCode.None;
            return resPacket;
        }

        [Route("Request/SignIn")]
        [HttpPost]
        public async Task<LoginRes> SignInRequest(LoginReq signInPacket)
        {
            var resPacket = new LoginRes();

            // 유저 패스워드를 암호화 한다.
            var encryptedPassword = Util.Encrypter.EncryptString(signInPacket.UserPw);

            Console.WriteLine($"Id({signInPacket.UserId}), Pw({encryptedPassword}), Send SignIn Request");

            // 회원가입이 가능한 정보인지를 DB 서버에 조회한다.
            var userValidationReq = new DBServer.UserValidationReq();
            userValidationReq.UserId = signInPacket.UserId;
            userValidationReq.EncryptedUserPw = encryptedPassword;

            var userValidationRes = await Util.HttpMessenger.RequestHttp<DBServer.UserValidationReq, DBServer.UserValidationRes>(
                "http://localhost:20000/", "DB/UserValidation", userValidationReq);

            // 회원가입이 불가능하다면, 이유를 적고 반환해준다.
            if (userValidationRes.Result != (short)ErrorCode.None)
            {
                resPacket.Result = userValidationRes.Result;
                resPacket.Token = -1;
                return resPacket;
            }

            // 회원가입이 가능하다면 DB 서버에 회원 가입을 요청한다.
            var userSignInReq = new DBServer.UserSignInReq();
            userSignInReq.UserId = signInPacket.UserId;
            userSignInReq.EncryptedUserPw = encryptedPassword;

            var userSignInRes = await Util.HttpMessenger.RequestHttp<DBServer.UserSignInReq, DBServer.UserSignInRes>(
                "http://localhost:20000", "DB/AddUser", userSignInReq);

            // 회원가입이 실패했다면, 이유를 적고 반환해준다.
            if (userSignInRes.Result != (short)ErrorCode.None)
            {
                resPacket.Result = userSignInRes.Result;
                resPacket.Token = -1;
                return resPacket;
            }

            // 토큰을 생성하고 DB 서버에 등록한다.
            resPacket.Token = TokenGenerator.GetInstance().CreateToken();

            var tokenAuthReq = new DBServer.TokenAuthReq();
            tokenAuthReq.UserId = signInPacket.UserId;
            tokenAuthReq.Token = resPacket.Token;

            var tokenAuthRes = await Util.HttpMessenger.RequestHttp<DBServer.TokenAuthReq, DBServer.TokenAuthRes>(
                "http://localhost:20000/", "DB/RegistToken", tokenAuthReq);

            // 토큰 등록이 실패했다면 에러 반환.
            if (tokenAuthRes.Result != (short)ErrorCode.None)
            {
                resPacket.Result = userValidationRes.Result;
                resPacket.Token = -1;
                return resPacket;
            }

            // 모든 절차가 완료되었다면 정상값을 반환한다.
            resPacket.Result = (short)ErrorCode.None;
            return resPacket;
        }

        //[Route("Request/Logout")]
        //[HttpPost]
        //public async Task<LogoutRes> LogoutRequest(LogoutReq logoutPacket)
        //{
        //	var resPacket = new LogoutRes();

        //	Console.WriteLine($"Id({logoutPacket.UserId}), Send Logout Packet");

        //	// Id가 MongoDB에 없다면 입력값 에러 반환.
        //	var isIdIsValid = await DB.MongoDbManager.GetIdExist(logoutPacket.UserId);
        //	if (isIdIsValid == false)
        //	{
        //		Console.WriteLine($"Id({logoutPacket.UserId}) is Invalid Id");
        //		resPacket.Result = (short)ErrorCode.LogoutInvalidId;
        //		return resPacket;
        //	}

        //	// Token이 Redis에 없다면 입력값 에러 반환.
        //	var isTokenValid = await DB.AuthTokenManager.CheckAuthToken(logoutPacket.UserId, logoutPacket.Token);
        //	if (isTokenValid == false)
        //	{
        //		Console.WriteLine($"Id({logoutPacket.UserId})'s Token({logoutPacket.Token}) is Invalid");
        //		resPacket.Result = (short) ErrorCode.LogoutInvalidToken;
        //		return resPacket;
        //	}

        //	// 입력 요청값이 Valid하다고 판단되면 Redis에서 값 삭제.
        //	await DB.AuthTokenManager.DeleteAuthToken(logoutPacket.UserId);
        //	resPacket.Result = (short) ErrorCode.None;
        //	Console.WriteLine($"Id({logoutPacket.UserId}), Logout Success");

        //	return resPacket;
        //}
    }
}
