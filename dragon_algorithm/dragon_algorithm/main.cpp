#include <iostream>
#include "CFG.h"
#include <stdlib.h>
#include <stdio.h>
#include "Lex.h"
#include "LL1.h"
#include <memory>
#include <Windows.h>
#include <gdiplus.h>

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

using std::cin;
using std::cout;

void TestParse()
{
	// 产生式列表
	const char *products = "S -> E\n"
						   "E -> E + T | E - T | T\n"
						   "T -> T * F | T / F | F\n"
						   "F -> (E) | num";

	// 非终结符号集合
	std::set<std::string> n_terminal_set =  { "S", "E", "T", "F" };

	// 终结符号集合
	std::set<std::string> terminal_set = { "num", "+", "-", "*", "/", "(", ")" };

	try
	{
		CFG::CFG cfg(n_terminal_set, terminal_set, products, "S");
		std::cout << "消除左递归之前: " << std::endl << cfg << std::endl;
		std::cout << "消除左递归后: " << std::endl << cfg.RemoveRecursive();
	}
	catch (const std::exception &exp)
	{
		std::cout << exp.what() << std::endl;
	}

	
}

void TestFirst()
{

}

// 获取树的深度
int ASTDeep()
{
	return 0;
}


// 获取树的宽度
int ASTWidth(ASTNode *node)
{
	if (node->nodes.size() == 0)
	{
		return 1;
	}

	int width = 0;
	for (auto n : node->nodes)
	{
		width += ASTWidth(n);
	}
	return width;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

const int kNodeWidth = 50;
const int kNodeHeight = 50;
const int space = 20;
const int kRadii = kNodeWidth / 2;
const int nBitmapW = 3000;
const int nBitmapH = 1080;

int AnsiToUnicode(LPWSTR wstrUnicode, LPCSTR szAnsi)
{
	DWORD dwMinSize = 0;
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, NULL, 0);
	if (0 == dwMinSize)
	{
		return 0;
	}
	MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, wstrUnicode, dwMinSize);

	return dwMinSize;
}

void MakeASTIMG(ASTNode *node, Gdiplus::Graphics &graphics, int center_x,  int deep = 0)
{
	FontFamily  fontFamily(L"Times New Roman");
	Font        font(&fontFamily, 18, FontStyleRegular, UnitPixel);
	int x = center_x - kRadii + kNodeWidth + space;
	
	
	Gdiplus::RectF       rectF(x , deep * 100, kNodeWidth, kNodeHeight);
	Gdiplus::SolidBrush  solidBrush(Color(255, 0, 0, 255));


	Pen      pen(Color::White, 1);
	graphics.DrawEllipse(&pen, rectF);

	Gdiplus::StringFormat sf;
	sf.SetAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	WCHAR wchar[100] = { 0 };
	AnsiToUnicode(wchar, node->symbolic->Name().c_str());

	graphics.DrawString(wchar, -1, &font, rectF, &sf, &solidBrush);

	int width = ASTWidth(node); // 当前节点宽度
	int sum_width = 0;
	int sum_width2 = 0;
	bool lastInMid = false;
	for (size_t i = 0; i < node->nodes.size(); i++)
	{
		int current_width = ASTWidth(node->nodes[i]);
		float pos = (float)i - (float)node->nodes.size() / 2 + 0.5;
		int cur_width = current_width * (kNodeWidth + space);
		int to_centor_x = 0;

		if (pos < 0) // 在左边
		{
			to_centor_x = center_x + (sum_width + cur_width / 2) * -1;
		}
		else if (pos == 0) { // 在中间
			to_centor_x = center_x;
			sum_width = (sum_width2 - width / 2) * (kNodeWidth + space);
			lastInMid = true;
		}
		else { // 在右边
			if (false == lastInMid) {
				sum_width = 0;
			}
			to_centor_x = center_x + sum_width + cur_width / 2;
		}

		
		graphics.DrawLine(&pen, x + kRadii, deep * 100 + kNodeHeight, to_centor_x  + kNodeWidth + space, (deep + 1) * 100);

		
		MakeASTIMG(node->nodes[i], graphics, to_centor_x, deep + 1);

		sum_width += cur_width;
		sum_width2 += current_width;
	}

}

// 生成语法树图片
void MakeASTIMG(ASTNode *node)
{
	///// 生成图片太麻烦了.... 还是先打印出来树结构吧
	//for (int i = 0; i <= deep; ++i)
	//{
	//	std::cout << "\t";
	//}
	//auto str = node->symbolic->Name() == "ε" ? "@" : node->symbolic->Name();

	//std::cout << str << std::endl << std::endl;;

	//
	//
	//for (size_t i = 0; i < node->nodes.size(); i++)
	//{
	//	MakeASTIMG(node->nodes[i], deep + 1);
	//}

	// 节点的宽度 高度 和间隔
	

	int width = ASTWidth(node); // 树的宽度

	HDC hMemoryDC = CreateCompatibleDC(NULL);

	HBITMAP hBitMap = CreateCompatibleBitmap(hMemoryDC, nBitmapW, nBitmapH);

	SelectObject(hMemoryDC, hBitMap);

	Graphics graphics(hMemoryDC);
	
	Gdiplus::SolidBrush white(Color(255, 255, 255, 255));
	graphics.FillRectangle(&white, 0, 0, nBitmapW, nBitmapH);

	MakeASTIMG(node, graphics, nBitmapW / 2);

	// 生成图片
	BITMAP bm;
	GetObject(hBitMap, sizeof(BITMAP), &bm);
	WORD BitsPerPixel = bm.bmBitsPixel;

	using namespace Gdiplus;
	Bitmap* bitmap = Bitmap::FromHBITMAP(hBitMap, NULL);
	EncoderParameters encoderParameters;
	ULONG compression;
	CLSID clsid;

	if (BitsPerPixel == 1)
	{
		compression = EncoderValueCompressionCCITT4;
	}
	else
	{
		compression = EncoderValueCompressionLZW;
	}
	GetEncoderClsid(L"image/jpeg", &clsid);

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderCompression;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;
	encoderParameters.Parameter[0].Value = &compression;

	bitmap->Save(L"1.jpeg", &clsid, &encoderParameters);
	delete bitmap;


}

void TestParseLL1()
{
	// 产生式列表
	const char *products = "S -> E\n"
		"E -> E + T | E - T | T\n"
		"T -> T * F | T / F | F\n"
		"F -> (E) | num";

	// 非终结符号集合
	std::set<std::string> n_terminal_set = { "S", "E", "T", "F" };

	// 终结符号集合
	std::set<std::string> terminal_set = { "num", "+", "-", "*", "/", "(", ")" };
	CFG::CFG cfg(n_terminal_set, terminal_set, products, "S");

	char line[255] = { 0 };
	cin.getline(line, sizeof(line));
	LL1 ll1(&cfg, new ExpLex(line));
	try
	{
		auto ast = ll1.Parse(); // 解析成功生成语法分析树
		MakeASTIMG(ast.root);
	}
	catch (std::exception &err)
	{
		std::cout << err.what() << std::endl;
	}
	

}

int main()
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	//TestParse();
	TestParseLL1();
	system("pause");
	
	return 0;
}