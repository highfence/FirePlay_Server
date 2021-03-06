﻿using Microsoft.Owin.Hosting;
using System;
using System.Threading.Tasks;

namespace LoginServer
{
	internal class Program
	{
		private static void Main(string[] args)
		{
#if DEBUG
            const string baseAddress = "http://localhost:19000/";
#else
            const string baseAddress = "http://*:19000/";
#endif

			using (WebApp.Start<Startup>(url: baseAddress))
			{
                LoginServerMain.Init();
                
				// TODO :: 로그인 서버 설정파일 읽어오기.
				Console.WriteLine("Login Server 실행 중 : " + baseAddress);
				Console.ReadLine();
			}
		}
	}
}
