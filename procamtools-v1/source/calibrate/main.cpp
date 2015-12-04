//
// Copyright (c) 2009-2011  Shuntaro Yamazaki (shun-yamazaki (at) aist.go.jp) 
//  and the National Institute of Advanced Industrial Science and Technology   
//
// $Id: main.cpp 4590 2011-05-30 22:13:45Z shun $
//

#include "stdafx.h"

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.Data.dll>
//using namespace System::Windows::Forms;


#include "MyForm.h"
using namespace calibrate;

//Yang:  when you create a Windows Forms project; the IDE does below for you automatically.
//In basic Windows Forms apps it's unlikely you'll ever need to even touch that file.
[STAThreadAttribute]
int __stdcall WinMain()
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	// Crear la ventana principal y ejecutarla
	Application::Run(gcnew calibrate::MyForm());
}


