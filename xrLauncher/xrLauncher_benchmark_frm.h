#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace xrLauncher
{
	/// <summary> 
	/// Summary for xrLauncher_benchmark_frm
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public __gc class xrLauncher_benchmark_frm : public System::Windows::Forms::Form
	{
	public: 
		xrLauncher_benchmark_frm(void)
		{
			InitializeComponent();
			m_test_cmds = new ArrayList();
		}
   	void Init();
	bool check_all_correct();
	void prepareBenchmarkFile(LPCSTR file_name);

	protected: 
		void Dispose(Boolean disposing)
		{
			if (disposing && components)
			{
				components->Dispose();
			}
			__super::Dispose(disposing);
		}
	private: ArrayList*							m_test_cmds;
	private: System::Windows::Forms::Panel *  panel1;
	private: System::Windows::Forms::CheckBox *  config1checkBox;
	private: System::Windows::Forms::CheckBox *  config2checkBox;
	private: System::Windows::Forms::CheckBox *  config3checkBox;
	private: System::Windows::Forms::CheckBox *  config4checkBox;
	private: System::Windows::Forms::ComboBox *  qualityComboBox;
	private: System::Windows::Forms::CheckBox *  nosoundCheckBox;
	private: System::Windows::Forms::Button *  runBenchmarkBtn;
	private: System::Windows::Forms::Button *  cancelBtn;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container* components;

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->panel1 = new System::Windows::Forms::Panel();
			this->config4checkBox = new System::Windows::Forms::CheckBox();
			this->config3checkBox = new System::Windows::Forms::CheckBox();
			this->config2checkBox = new System::Windows::Forms::CheckBox();
			this->config1checkBox = new System::Windows::Forms::CheckBox();
			this->qualityComboBox = new System::Windows::Forms::ComboBox();
			this->nosoundCheckBox = new System::Windows::Forms::CheckBox();
			this->runBenchmarkBtn = new System::Windows::Forms::Button();
			this->cancelBtn = new System::Windows::Forms::Button();
			this->panel1->SuspendLayout();
			this->SuspendLayout();
			// 
			// panel1
			// 
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel1->Controls->Add(this->config4checkBox);
			this->panel1->Controls->Add(this->config3checkBox);
			this->panel1->Controls->Add(this->config2checkBox);
			this->panel1->Controls->Add(this->config1checkBox);
			this->panel1->Location = System::Drawing::Point(8, 8);
			this->panel1->Name = S"panel1";
			this->panel1->Size = System::Drawing::Size(144, 104);
			this->panel1->TabIndex = 0;
			// 
			// config4checkBox
			// 
			this->config4checkBox->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->config4checkBox->Location = System::Drawing::Point(8, 80);
			this->config4checkBox->Name = S"config4checkBox";
			this->config4checkBox->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->config4checkBox->Size = System::Drawing::Size(120, 16);
			this->config4checkBox->TabIndex = 3;
			this->config4checkBox->Text = S"config4";
			this->config4checkBox->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// config3checkBox
			// 
			this->config3checkBox->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->config3checkBox->Location = System::Drawing::Point(8, 56);
			this->config3checkBox->Name = S"config3checkBox";
			this->config3checkBox->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->config3checkBox->Size = System::Drawing::Size(120, 16);
			this->config3checkBox->TabIndex = 2;
			this->config3checkBox->Text = S"config3";
			this->config3checkBox->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// config2checkBox
			// 
			this->config2checkBox->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->config2checkBox->Location = System::Drawing::Point(8, 32);
			this->config2checkBox->Name = S"config2checkBox";
			this->config2checkBox->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->config2checkBox->Size = System::Drawing::Size(120, 16);
			this->config2checkBox->TabIndex = 1;
			this->config2checkBox->Text = S"config2";
			this->config2checkBox->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// config1checkBox
			// 
			this->config1checkBox->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->config1checkBox->Location = System::Drawing::Point(8, 8);
			this->config1checkBox->Name = S"config1checkBox";
			this->config1checkBox->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->config1checkBox->Size = System::Drawing::Size(120, 16);
			this->config1checkBox->TabIndex = 0;
			this->config1checkBox->Text = S"config1";
			this->config1checkBox->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// qualityComboBox
			// 
			this->qualityComboBox->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->qualityComboBox->Location = System::Drawing::Point(168, 16);
			this->qualityComboBox->Name = S"qualityComboBox";
			this->qualityComboBox->Size = System::Drawing::Size(144, 21);
			this->qualityComboBox->TabIndex = 1;
			// 
			// nosoundCheckBox
			// 
			this->nosoundCheckBox->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->nosoundCheckBox->Location = System::Drawing::Point(168, 56);
			this->nosoundCheckBox->Name = S"nosoundCheckBox";
			this->nosoundCheckBox->Size = System::Drawing::Size(144, 16);
			this->nosoundCheckBox->TabIndex = 4;
			this->nosoundCheckBox->Text = S"no sound";
			this->nosoundCheckBox->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// runBenchmarkBtn
			// 
			this->runBenchmarkBtn->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->runBenchmarkBtn->Location = System::Drawing::Point(208, 128);
			this->runBenchmarkBtn->Name = S"runBenchmarkBtn";
			this->runBenchmarkBtn->Size = System::Drawing::Size(104, 26);
			this->runBenchmarkBtn->TabIndex = 5;
			this->runBenchmarkBtn->Text = S"Run benchmark";
			this->runBenchmarkBtn->Click += new System::EventHandler(this, runBenchmarkBtn_Click);
			// 
			// cancelBtn
			// 
			this->cancelBtn->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->cancelBtn->Location = System::Drawing::Point(80, 128);
			this->cancelBtn->Name = S"cancelBtn";
			this->cancelBtn->Size = System::Drawing::Size(104, 26);
			this->cancelBtn->TabIndex = 6;
			this->cancelBtn->Text = S"Cancel";
			// 
			// xrLauncher_benchmark_frm
			// 
			this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
			this->ClientSize = System::Drawing::Size(320, 158);
			this->Controls->Add(this->cancelBtn);
			this->Controls->Add(this->runBenchmarkBtn);
			this->Controls->Add(this->nosoundCheckBox);
			this->Controls->Add(this->qualityComboBox);
			this->Controls->Add(this->panel1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Name = S"xrLauncher_benchmark_frm";
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = S"xrLauncher_benchmark_frm";
			this->panel1->ResumeLayout(false);
			this->ResumeLayout(false);

		}		
	private: System::Void runBenchmarkBtn_Click(System::Object *  sender, System::EventArgs *  e);

};
}