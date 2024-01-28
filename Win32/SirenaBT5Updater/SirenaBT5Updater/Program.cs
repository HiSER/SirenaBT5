using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace SirenaBT5Updater
{
	static class Program
	{
		/// <summary>
		/// Главная точка входа для приложения.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			if (args.Length == 0)
			{
				Application.Run(new MainForm(null));
			}
			else
			{
				Application.Run(new MainForm(args[0]));
			}
		}
	}
}
