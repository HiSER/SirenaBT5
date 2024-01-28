using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace SirenaBT5Updater
{
	public class JSON
	{
		public static int GetInt(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":([0-9-]+)")).Match(json);
			string value = m.Groups[1].ToString();
			return Convert.ToInt32(value);
		}
		public static double GetDouble(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":([0-9-\\.]+)")).Match(json);
			string value = m.Groups[1].ToString();
			//return Convert.ToDouble(value);
			return double.Parse(value, System.Globalization.CultureInfo.InvariantCulture);
		}
		public static string GetString(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":\"([0-9a-zA-Z+/=]*)\"")).Match(json);
			string value = m.Groups[1].ToString();
			return value;
		}
		public static string GetStringBase64(string name, string json)
		{
			byte[] b = Convert.FromBase64String(GetString(name, json));
			string value = Encoding.UTF8.GetString(b);
			return value;
		}
		public static bool GetBool(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":(true|false)")).Match(json);
			string value = m.Groups[1].ToString();
			return (value == "true");
		}
		public static string GetArray(string name, string json)
		{
			Match m = (new Regex("\"" + name + "\":\\[([^\\]]*)\\]")).Match(json);
			string value = m.Groups[1].ToString();
			return value;
		}
		public static List<int> GetArrayIntItems(string array)
		{
			List<int> result = new List<int>();
			foreach (Match m in (new Regex(@"\d+")).Matches(array))
			{
				result.Add(Convert.ToInt32(m.ToString()));
			}
			return result;
		}
		public static List<string> GetArrayObjectItems(string array)
		{
			List<string> result = new List<string>();
			foreach (Match m in (new Regex(@"\{([^\}]+)\}")).Matches(array))
			{
				result.Add(m.ToString());
			}
			return result;
		}
	}
}
