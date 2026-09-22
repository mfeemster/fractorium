namespace apoconv;

public partial class Form1 : Form
{
	private readonly List<string> m_CalcFuncLines = [];
	private readonly List<string> m_FinalCalcFuncLines = [];
	private readonly List<ParamInfo> m_Params = [];
	private readonly List<string> m_PrecalcFuncLines = [];
	private readonly List<PrecalcParamInfo> m_StructParams = [];

	private readonly SortedDictionary<string, string> replacements = new()
	{
		{ "(double)", "" },
		{ "(float)", "" },
		{ "double", "T" },
		{ "float", "T" },
		{ "Complex", "std::complex<T>" },
		//{ "0.0", "0" },
		{ "0.5", "T(0.5)" },
		{ "1.0", "T(1.0)" },
		{ "1.5", "T(1.5)" },
		{ "2.0", "T(2.0)" },
		{ "3.0", "T(3.0)" },
		{ "0.1", "T(0.1)" },
		{ "0.01", "T(0.01)" },
		{ "0.001", "T(0.001)" },
		{ "12.9898", "T(12.9898)" },
		{ "78.233", "T(78.233)" },
		{ "43758.5453", "T(43758.5453)" },
		{ "FTx", "helper.In.x" },
		{ "FTy", "helper.In.y" },
		{ "FTz", "helper.In.z" },
		{ "FPx", "helper.Out.x" },
		{ "FPy", "helper.Out.y" },
		{ "FPz", "helper.Out.z" },
		{ "VVAR", "m_Weight" },
		{ "floor(", "Floor<T>(" },
		{ "ceil(", "std::ceil(" },
		{ "round(", "std::round(" },
		{ "sin(", "std::sin(" },
		{ "sinh(", "std::sinh(" },
		{ "asin(", "std::asin(" },
		{ "asinh(", "std::asinh(" },
		{ "cos(", "std::cos(" },
		{ "cosh(", "std::cosh(" },
		{ "acos(", "std::acos(" },
		{ "acosh(", "std::acosh(" },
		{ "tan(", "std::tan(" },
		{ "tanh(", "std::tanh(" },
		{ "atan(", "std::atan(" },
		{ "atan2(", "std::atan2(" },
		{ "atanh(", "std::atanh(" },
		{ "sqrt(", "std::sqrt(" },
		{ "pow(", "std::pow(" },
		{ "fabs(", "std::abs(" },
		{ "abs(", "std::abs(" },
		{ "log(", "std::log(" },
		{ "exp(", "std::exp(" },
		{ "sqr(", "Sqr(" },
		{ "random01()", "rand.Frand01<T>()" },
		{ "M_E", "T(M_E)" },
		{ "M_LOG2E", "T(M_LOG2E)" },
		{ "M_LOG10E", "T(M_LOG10E)" },
		{ "M_LN2", "T(M_LN2)" },
		{ "M_LN10", "T(M_LN10)" },
		{ "M_PI", "T(M_PI)" },
		{ "M_PI_2", "T(M_PI_2)" },
		{ "M_PI_4", "M_PI4" },
		{ "M_1_PI", "T(M_1_PI)" },
		{ "M_2_PI", "T(M_2_PI)" },
		{ "M_2_SQRTPI", "T(M_2_SQRTPI)" },
		{ "M_SQRT2", "T(M_SQRT2)" },
		{ "M_SQRT1_2", "T(M_SQRT1_2)" },
	};

	public Form1() => InitializeComponent();//var lines = File.ReadAllLines("./waves4.cpp");//textBox1.Lines = lines;

	private string Apo2Fractorium(string[] lines, bool doexport)
	{
		if (lines.Length == 0)
			return "";

		//doexport = false;
		var isparvar = false;
		var pluginname = "";
		var varclassname = "";
		m_Params.Clear();
		m_StructParams.Clear();
		m_PrecalcFuncLines.Clear();
		m_CalcFuncLines.Clear();
		m_FinalCalcFuncLines.Clear();
		var structstart = false;

		foreach (var line in lines)
		{
			if (line.Contains("APO_PLUGIN"))
			{
				var splits = line.Split(new char[] { '\"' });

				if (splits.Length >= 1)
				{
					pluginname = splits[1];
					varclassname = pluginname.FirstCharToUpper() + "Variation";
				}
			}
		}

		foreach (var line in lines)
		{
			if (line.Contains("typedef struct"))
			{
				structstart = true;
				continue;
			}

			if (structstart)
			{
				if (line.Contains("{"))
					continue;

				if (line.Contains("}"))
					break;

				var splits = line.Trim().Split(new string[] { " ", ",", ";", "(", ")", "double", "float", "int", "unsigned", "long", "short", "char" }, StringSplitOptions.RemoveEmptyEntries);

				foreach (var split in splits)
				{
					m_StructParams.Add(new PrecalcParamInfo
					{
						OrigName = split.Trim(),
						Name = split.Trim(),
						MemberName = "m_" + split.RemoveFirst(pluginname + "_").FirstCharToUpper().Replace("_", "").Trim(),
					});
					isparvar = true;
				}
			}
		}

		var paramsstart = false;

		foreach (var line in lines)
		{
			if (line.Contains("APO_VARIABLES("))
			{
				paramsstart = true;
				continue;
			}

			if (paramsstart)
			{
				if (line.Contains(");"))
					break;

				var splits = line.Split(new char[] { '(', ')', ',' });

				if (splits.Length > 2)
				{
					var pt = "";
					var l = "";
					var u = "";
					var def = "";

					if (line.Contains("VAR_INTEGER_RANGE"))
					{
						pt = "eParamType::INTEGER";

						if (splits.Length >= 5)
						{
							l = splits[2].Trim();
							u = splits[3].Trim();
							def = splits[4].Trim();
						}
					}
					else if (line.Contains("VAR_REAL_RANGE"))
					{
						pt = "eParamType::REAL";

						if (splits.Length >= 5)
						{
							l = splits[2].Trim();
							u = splits[3].Trim();
							def = splits[4].Trim();
						}
					}
					else if (line.Contains("VAR_REAL_CYCLE"))
					{
						pt = "eParamType::REAL_CYCLIC";

						if (splits.Length >= 5)
						{
							l = splits[2].Trim();
							u = splits[3].Trim();
							def = splits[4].Trim();
						}
					}
					else if (line.Contains("VAR_INTEGER_NONZERO"))
					{
						pt = "eParamType::INTEGER_NONZERO";
						def = splits[2].Trim();
					}
					else if (line.Contains("INTEGER("))
					{
						pt = "eParamType::INTEGER";
						def = splits[2].Trim();
					}
					else
						def = splits[2].Trim();

					m_Params.Add(new ParamInfo
					{
						Name = splits[1].Trim(),
						MemberName = "m_" + splits[1].RemoveFirst(pluginname + "_").FirstCharToUpper().Replace("_", "").Trim(),
						Default = def,
						Lower = l,
						Upper = u,
						ParamType = pt
					});
					isparvar = true;
				}
			}
		}

		foreach (var param in m_Params)
		{
			if (m_StructParams.Any(pi => pi.Name == param.Name))
				_ = m_StructParams.RemoveAll(pi => pi.Name == param.Name);
		}

		foreach (var param in m_StructParams)
		{
			if (!param.Name.StartsWith(pluginname + "_"))
				param.Name = pluginname + "_" + param.Name;

			if (!param.Name.EndsWith("_precalc"))
				param.Name += "_precalc";

			if (!param.MemberName.EndsWith("Precalc"))
				param.MemberName += "Precalc";
		}

		var calcstart = false;
		var bracketcount = 0;

		foreach (var line in lines)
		{
			if (line.Contains("PluginVarCalc"))
			{
				calcstart = true;
				continue;
			}

			if (calcstart)
			{
				if (line.Contains("{"))
					bracketcount++;

				if (line.Contains("}"))
					bracketcount--;

				if (bracketcount <= 0)
					break;

				if (line != "{" && !line.Contains("return"))
				{
					var templine = line;

					if (line.Contains("FPx") || line.Contains("FPy") || line.Contains("FPz"))
					{
						templine = templine.Replace("+=", "=");
						templine = templine.Replace("-=", "= -(");//Will cause an error, forcing manual inspection
					}

					m_CalcFuncLines.Add(templine.Trim());
				}
			}
		}

		var precalcstart = false;
		bracketcount = 0;

		foreach (var line in lines)
		{
			if (line.Contains("PluginVarPrepare"))
			{
				precalcstart = true;
				continue;
			}

			if (precalcstart)
			{
				if (line.Contains("{"))
					bracketcount++;

				if (line.Contains("}"))
					bracketcount--;

				if (bracketcount <= 0)
					break;

				if (line != "{" && !line.Contains("return"))
				{
					var templine = line;

					foreach (var rep in replacements)
						templine = templine.Replace(rep.Key, rep.Value);

					m_PrecalcFuncLines.Add(templine.Trim());
				}
			}
		}

		var final = "/// <summary>\r\n";
		final += "/// " + pluginname + ".\r\n";
		final += "/// </summary>\r\n";
		final += "template <typename T>\r\n";

		if (isparvar)
			final += "class " + varclassname + " : public ParametricVariation<T>\r\n";
		else
			final += "class " + varclassname + " : public Variation<T>\r\n";

		final += "{\r\n";
		final += "public:\r\n";

		if (isparvar)
			final += "\t" + varclassname + "(T weight = 1.0) : ParametricVariation<T>(\"" + pluginname + "\", eVariationId::VAR_" + pluginname.ToUpper() + ", weight)\r\n";
		else
			final += "\t" + varclassname + "(T weight = 1.0) : Variation<T>(\"" + pluginname + "\", eVariationId::VAR_" + pluginname.ToUpper() + ", weight)\r\n";

		final += "\t{\r\n";

		if (isparvar)
			final += "\t	Init();\r\n";

		final += "\t}\r\n\r\n";

		if (isparvar)
			final += "\tPARVARCOPY(" + varclassname + ")\r\n";
		else
			final += "\tVARCOPY(" + varclassname + ")\r\n";

		final += "\r\n\tvirtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override\r\n";
		final += "\t{\r\n";

		foreach (var line in m_CalcFuncLines)
		{
			var templine = "\t\t" + line;

			foreach (var rep in replacements)
				templine = templine.Replace(rep.Key, rep.Value);

			templine += "\r\n";

			foreach (var param in m_Params)
				templine = templine.Replace("VAR(" + param.Name + ")", param.MemberName);

			foreach (var param in m_StructParams)
				templine = templine.Replace("VAR(" + param.OrigName + ")", param.MemberName);

			m_FinalCalcFuncLines.Add(templine.Trim());
			final += templine;
		}

		final += "\t\thelper.Out.z = DefaultZ(helper);\r\n";
		final += "\t}\r\n";
		final += "\r\n\tvirtual string OpenCLString() const override\r\n";
		final += "\t{\r\n";
		final += "\t	ostringstream ss, ss2;\r\n";
		final += "\t	intmax_t i = 0, varIndex = IndexInXform();\r\n";
		final += "\t	ss2 << \"_\" << XformIndexInEmber() << \"]\";\r\n";
		final += "\t	string index = ss2.str();\r\n";
		final += "\t	string weight = WeightDefineString();\r\n";

		foreach (var param in m_Params)
		{
			final += "\t\tstring " + param.MemberName.Replace("m_", "").ToLower() + " = \"parVars[\" + ToUpper(m_Params[i++].Name()) + index;\r\n";
		}

		foreach (var param in m_StructParams)
		{
			final += "\t\tstring " + param.MemberName.Replace("m_", "").ToLower() + " = \"parVars[\" + ToUpper(m_Params[i++].Name()) + index;\r\n";
		}

		final += "\t\tss << \"\\t{\\n\"\r\n";

		foreach (var line in m_FinalCalcFuncLines)
		{
			var templine = line != "" ? "\t\t<< \"\\t\\t" + line + "\\n\"\r\n" : "\t\t<< \"\\n\"\r\n";

			templine = templine.Replace("\\tT ", "\\treal_t ").
					   Replace("helper.In.x", "vIn.x").
					   Replace("helper.In.y", "vIn.y").
					   Replace("helper.In.z", "vIn.z").
					   Replace("helper.Out.x", "vOut.x").
					   Replace("helper.Out.y", "vOut.y").
					   Replace("helper.Out.z", "vOut.z").
					   Replace("Floor<T>", "floor").
					   Replace("std::abs", "fabs").
					   Replace("m_Weight", "\" << weight << \"").
					   Replace("std::", "").
					   Replace("rand.Frand01<T>()", "MwcNext01(mwc)").
					   Replace("T(", "(real_t)(")
					   ;

			foreach (var param in m_Params)
			{
				if (templine.Contains(param.MemberName))
					templine = templine.Replace(param.MemberName, "\" << " + param.MemberName[2..].ToLower() + " << \"");
			}

			foreach (var param in m_StructParams)
			{
				if (templine.Contains(param.MemberName))
					templine = templine.Replace(param.MemberName, "\" << " + param.MemberName[2..].ToLower() + " << \"");
			}

			final += templine;
		}

		final += "\t\t<< \"\\t\\tvOut.z = \" << DefaultZCl()\r\n";
		final += "\t\t<< \"\\t}\\n\";\r\n";
		final += "\t\treturn ss.str();\r\n";
		final += "\t}\r\n";

		if (isparvar)
		{
			final += "\r\n\tvirtual void Precalc() override\r\n";
			final += "\t{\r\n";

			foreach (var line in m_PrecalcFuncLines)
			{
				var templine = line;

				foreach (var param in m_Params)
					templine = templine.Replace("VAR(" + param.Name + ")", param.MemberName);

				foreach (var param in m_StructParams)
					templine = templine.Replace("VAR(" + param.OrigName + ")", param.MemberName);

				final += "\t\t" + templine + "\r\n";
			}

			final += "\t}\r\n";
		}

		final += "\r\n\tvirtual vector<string> OpenCLGlobalFuncNames() const override\r\n";
		final += "\t{\r\n";
		final += "\t	return vector<string> { \"Zeps\" };\r\n";
		final += "\t}\r\n";
		var saidprecalc = false;

		if (isparvar)
		{
			final += "\r\nprotected:\r\n";
			final += "\tvoid Init()\r\n";
			final += "\t{\r\n";
			final += "\t\tstring prefix = Prefix();\r\n";
			final += "\t\tm_Params.clear();\r\n";

			foreach (var param in m_Params)
				final += "\t\tm_Params.push_back(ParamWithName<T>(&" + param.MemberName
						 + ", prefix + \""
						 + param.Name
						 + ((param.Default != "0" && param.Default != "0.0" && param.Default != "") || param.ParamType != ""
							? "\", T(" + param.Default + ")"

							: "\"")
						 + (param.ParamType != "" ? ", " + param.ParamType : "")
						 + (param.Lower != "" ? ", T(" + param.Lower + ")" : "")
						 + (param.Upper != "" ? ", T(" + param.Upper + ")" : "")
						 + "));\r\n";

			foreach (var param in m_StructParams)
			{
				final += "\t\tm_Params.push_back(ParamWithName<T>(true, &" + param.MemberName
						 + ", prefix + \""
						 + param.Name + "\"));" + (!saidprecalc ? "//Precalc." : "") + "\r\n";
				saidprecalc = true;
			}

			final += "\t}\r\n";
			final += "\r\nprivate:\r\n";

			foreach (var param in m_Params)
				final += "\tT " + param.MemberName + ";\r\n";

			saidprecalc = false;

			foreach (var param in m_StructParams)
			{
				final += "\tT " + param.MemberName + ";" + (!saidprecalc ? "//Precalc." : "") + "\r\n";
				saidprecalc = true;
			}
		}

		final += "};\r\n\r\n";

		if (doexport)
		{
			if (isparvar)
				final += "MAKEPREPOSTPARVAR(" + pluginname.FirstCharToUpper() + ", " + pluginname + ", " + pluginname.ToUpper() + ")\r\n";
			else
				final += "MAKEPREPOSTVAR(" + pluginname.FirstCharToUpper() + ", " + pluginname + ", " + pluginname.ToUpper() + ")\r\n";

			final += "EXPORTPREPOSTREGVAR(" + pluginname.FirstCharToUpper() + ", T) \\\r\n";
			final += "ADDPREPOSTREGVAR(" + pluginname.FirstCharToUpper() + ")\r\n\r\n";
		}

		return final;
	}

	private void textBox1_DragDrop(object? sender, DragEventArgs e)
	{
		textBox1.TextChanged -= textBox1_TextChanged;
		var final = "";
		List<string> tb1lines = [];

		if (e.Data?.GetDataPresent(DataFormats.FileDrop) == true)
		{
			if (e.Data.GetData(DataFormats.FileDrop) is string[] files)
			{
				foreach (var file in files)
				{
					var lines = System.IO.File.ReadAllLines(file);
					tb1lines.AddRange(lines.ToList());
					try
					{
						final += Apo2Fractorium(lines, true);
					}
					catch (Exception ex)
					{
						_ = MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
						break;
					}
					/*

										var guid = Guid.NewGuid();
										var fileonly = System.IO.Path.GetFileName(file);
										final += "<Component Id=\"" + fileonly + "\" Guid=\"" + guid + "\">\n";
										final += "\t<File Id=\"" + fileonly + "\" Source = \"$(var.SolutionDir)..\\..\\..\\Data\\" + fileonly + "\" KeyPath=\"yes\" Checksum=\"yes\" ReadOnly=\"yes\"/>\n";
										final += "</Component>\n\n";
					*/
				}

				textBox1.Lines = tb1lines.ToArray();
				textBox2.Text = final;
			}
		}

		textBox1.TextChanged += textBox1_TextChanged;
	}

	private void textBox1_DragEnter(object sender, DragEventArgs e) => e.Effect = DragDropEffects.Copy;

	private void textBox1_TextChanged(object? sender, EventArgs e)
	{
		try
		{
			textBox2.Text = Apo2Fractorium(textBox1.Lines, true);
		}
		catch (Exception ex)
		{
			_ = MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
		}
	}
}

public class ParamInfo
{
	public string Default = "";
	public string Lower = "";
	public string MemberName = "";
	public string Name = "";
	public string ParamType = "";
	public string Upper = "";
}

public class PrecalcParamInfo : ParamInfo
{
	public string OrigName = "";
}

public static class StringExtensions
{
	public static string FirstCharToUpper(this string input)
	{
		return input switch
		{
			null => throw new ArgumentNullException(nameof(input)),
			"" => throw new ArgumentException($"{nameof(input)} cannot be empty", nameof(input)),
			_ => input.First().ToString().ToUpper() + input[1..],
		};
	}

	public static string RemoveFirst(this string input, string removeString)
	{
		var index = input.IndexOf(removeString);
		return (index < 0)
			   ? input
			   : input.Remove(index, removeString.Length);
	}
}