#include "stdafx.h"
#include "Kolko_i_Krzyzyk.h"
#include "time.h"
#include <vector>
#include <iostream>
#include <string>

using namespace std;

#define GAME_WINDOW_WIDTH 500
#define GAME_WINDOW_HEIGHT 600

#define MAX_LOADSTRING 100

#define FIELD_SIZE 50 // wielkosc przycisku pola
#define FIELD_PADDING 5 // odstep miedzy przyciskami

//klasa pola gry trzyma w sobie przycisk, rozmiar (RECT) przycisku, pozycje x i y a takze co sie w nim znajduje 0 albo 1 albo - 1
class GameField
{
public:
	GameField(HWND button, RECT imageRect, int x, int y)
	{
		this->button = button;
		this->imageRect = imageRect;
		this->x = x;
		this->y = y;
	};
	HWND button;//przycisk
	RECT imageRect;//wspolrzedne przycisku a takze i bitmapy
	int x, y;
	int symbol = 0;//wartosc pola, albo 0 - puste, albo -1 albo 1
};

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name



int gameSetupStep = 0; //uzywane do okreselenia ile 
					   //etapow ustawiania gry uzytkownik przeszedl

GameField* fields[3][3]; //tablica z polami gry 3x3
int gameResult = 0; // wynik gry, jesli 0 oznacza ze gra sie nie skonczyla
					// jesli -1 wygral gracz 2
					// jesli 1 wygral gracz 1
int usedFields = 0; //uzyte pola uzywane tez do okreslenia numeru rundy, 
				    //zwieksza sie po kazdym kliknieciu w przycisk pola
BOOL player1Turn = TRUE; //flaga ktora mowi czyj teraz jest ruch

vector<string> symbolsFromData;

//index wybranego przez gracza #1 symbolu
int player1Symbol = 0;
//index wybranego przez gracza #1 koloru
int player1Color = 0;
//sciezka do obrazka
string pathToPlayer1Symbol = "";


//index wybranego przez gracza #2 symbolu
int player2Symbol = 1;
//sciezka do obrazka
string pathToPlayer2Symbol = "";
//index wybranego przez gracza #2 koloru
int player2Color = 1;
//tablica z mozliwymi kolorami do wyboru
vector<COLORREF> Symbol_Colors =
{
	0xFF6666,  //czerowny
	0x00FF00,  //zielony
	0x0000FF,  //niebieski
	0xFF00FF,  //różowy
	0xFFFF00,  //żółty
	0x00FFFF   //turkusowy
};

BOOLEAN doDelay = FALSE;

//pobiera wszystkie obrazki o rozszerzeniu bmp z danej sciezki
void GetSymbolsFromFolder()
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile("symbols\\*", &ffd);
	symbolsFromData.clear();
	do
	{
		if (hFind != INVALID_HANDLE_VALUE && strcmp(ffd.cFileName, ".") && strcmp(ffd.cFileName, ".."))
		{
			string stringed = ffd.cFileName;
			if (stringed.substr(stringed.find_last_of('.') + 1).compare("bmp") == 0)
			{
				symbolsFromData.push_back(stringed.substr(0, stringed.find_last_of('.')));
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);


}
//skleja stringa z sciezka dodaje do niego rozszerzenie i folder w ktorym powinien sie znajdowac
//np podajemy mu kolko a on robi z tego sciezke do pliku symbols/kolko.bmp
string GetPathToSymbol(string s)
{
	return "symbols//" + s + ".bmp";
}

//robi z pierwszej litery wyrazu wielka litere jesli sie da
string FirstToUpper(string s)
{
	if ((int)s[0] >= 97 && (int)s[0] <= 122)
	{
		return (char)((int)s[0] - 32) + s.substr(1);
	}
	return s;
}

void CheckForEndGame();
void ArticifalMove();

HWND hWnd;

HWND hWnd_TitleTxt, hWnd_ModeTxt, hWnd_SettingsTxt;


HWND hWnd_ModeButtonGVG, hWnd_ModeButtonGVK;
HWND hWnd_PlayerTxt1, hWnd_PlayerTxt2;
HWND hWnd_ComboSymbols1, hWnd_ComboSymbols2;

HWND hWnd_PreviousColor1, hWnd_NextColor1;
HWND hWnd_PreviousColor2, hWnd_NextColor2;

HWND hWnd_CheckBoxPrimacy1, hWnd_CheckBoxPrimacy2;//checkboxy ktore okreslaja kto ma zaczynac gre
HWND hWnd_StartButton;//przycisk rozpoczecia gry

HWND hWnd_GameStateTxt, hWnd_RoundTxt;

BOOL playerVsPlayerMode = FALSE;//mowi o tym jaki tryb gry zostal zalaczony

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_KOLKOIKRZYZYK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KOLKOIKRZYZYK));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_KOLKOIKRZYZYK);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindowW(szWindowClass, szTitle,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,//ustawiamy zeby nie dalo sie okienka zrobic na pelny ekran i zeby nie dalo sie modyfikowac jego wielkosci
		CW_USEDEFAULT, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);


	hWnd_TitleTxt = CreateWindowEx(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE |
		SS_CENTER, 0, 5, GAME_WINDOW_WIDTH, 20, hWnd, NULL, hInstance, NULL);
	SetWindowText(hWnd_TitleTxt, "Kółko i Krzyżyk");//tekst tytulowy na samej gorze


	hWnd_ModeTxt = CreateWindowEx(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE |
		SS_CENTER, 0, 45, GAME_WINDOW_WIDTH, 20, hWnd, NULL, hInstance, NULL);
	SetWindowText(hWnd_ModeTxt, "Wybierz tryb gry:");

	hWnd_ModeButtonGVG = CreateWindowEx(0, "BUTTON", "Gracz vs Gracz", WS_CHILD | WS_VISIBLE,
		(GAME_WINDOW_WIDTH - 320) / 2,
		75,
		150, 30, hWnd, NULL, hInstance, NULL);

	hWnd_ModeButtonGVK = CreateWindowEx(0, "BUTTON", "Gracz vs Komputer", WS_CHILD | WS_VISIBLE,
		(GAME_WINDOW_WIDTH - 320) / 2 + 160,
		75,
		150, 30, hWnd, NULL, hInstance, NULL);

	hWnd_SettingsTxt = CreateWindowEx(0, "STATIC", NULL, WS_CHILD |
		SS_CENTER, 0, 115, GAME_WINDOW_WIDTH, 20, hWnd, NULL, hInstance, NULL);
	SetWindowText(hWnd_SettingsTxt, "Ustaw symbole i kolory:");

	hWnd_PlayerTxt1 = CreateWindowEx(0, "STATIC", NULL, WS_CHILD |
		SS_CENTER, 50, 145, 150, 20, hWnd, NULL, hInstance, NULL);
	SetWindowText(hWnd_PlayerTxt1, "Gracz 1:");

	hWnd_PlayerTxt2 = CreateWindowEx(0, "STATIC", NULL, WS_CHILD |
		SS_CENTER, GAME_WINDOW_WIDTH - 200, 145, 150, 20, hWnd, NULL, hInstance, NULL);
	SetWindowText(hWnd_PlayerTxt2, "Gracz 2:");

	srand(time(NULL));//inicjalizacja losowosci

	hWnd_PreviousColor1 = CreateWindowEx(0, "BUTTON", "<<", WS_CHILD,
		70, 190, 25, 25, hWnd, NULL, hInstance, NULL);
	hWnd_NextColor1 = CreateWindowEx(0, "BUTTON", ">>", WS_CHILD,
		155, 190, 25, 25, hWnd, NULL, hInstance, NULL);
	ShowWindow(hWnd_PreviousColor1, SW_HIDE);

	hWnd_PreviousColor2 = CreateWindowEx(0, "BUTTON", "<<", WS_CHILD,
		GAME_WINDOW_WIDTH - 180, 190, 25, 25, hWnd, NULL, hInstance, NULL);
	hWnd_NextColor2 = CreateWindowEx(0, "BUTTON", ">>", WS_CHILD,
		GAME_WINDOW_WIDTH - 95, 190, 25, 25, hWnd, NULL, hInstance, NULL);


	hWnd_ComboSymbols1 = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", NULL, WS_CHILD | WS_BORDER |
		CBS_DROPDOWNLIST, 50, 240, 150, 200, hWnd, NULL, hInstance, NULL);
	hWnd_ComboSymbols2 = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", NULL, WS_CHILD | WS_BORDER |
		CBS_DROPDOWNLIST, GAME_WINDOW_WIDTH - 200, 240, 150, 200, hWnd, NULL, hInstance, NULL);

	GetSymbolsFromFolder();//pobieramy obrazki z folderu

	//wypelniamy oba comboboxy obrazkami (oboje gracze moga korzystac z tego samego zestawu znakow
	for (int i = 0; i < symbolsFromData.size(); ++i)
	{
		SendMessage(hWnd_ComboSymbols1, CB_ADDSTRING, 0, (LPARAM)FirstToUpper(symbolsFromData[i]).c_str());
		SendMessage(hWnd_ComboSymbols2, CB_ADDSTRING, 0, (LPARAM)FirstToUpper(symbolsFromData[i]).c_str());
	}
	SendMessage(hWnd_ComboSymbols1, CB_SETCURSEL, player1Symbol, 0);
	SendMessage(hWnd_ComboSymbols2, CB_SETCURSEL, player2Symbol, 0);

	hWnd_CheckBoxPrimacy1 = CreateWindowEx(0, "BUTTON", "Zaczyna pierwszy", WS_CHILD | BS_AUTOCHECKBOX,
		50, 270, 140, 30, hWnd, NULL, hInstance, NULL);
	hWnd_CheckBoxPrimacy2 = CreateWindowEx(0, "BUTTON", "Zaczyna pierwszy", WS_CHILD | BS_AUTOCHECKBOX,
		GAME_WINDOW_WIDTH - 200, 270, 140, 30, hWnd, NULL, hInstance, NULL);

	int randomling = rand() % 2;
	//losowo ustalamy ktory gracz ma zaczynac
	if (randomling == 0)
	{
		SendMessage(hWnd_CheckBoxPrimacy1, BM_SETCHECK, BST_CHECKED, 0);
	}
	else {
		SendMessage(hWnd_CheckBoxPrimacy2, BM_SETCHECK, BST_CHECKED, 0);
	}

	hWnd_StartButton = CreateWindowEx(0, "BUTTON", "Rozpocznij grę!", WS_CHILD,
		(GAME_WINDOW_WIDTH - 300) / 2,
		320,
		300, 30, hWnd, NULL, hInstance, NULL);

	hWnd_RoundTxt = CreateWindowEx(0, "STATIC", NULL, WS_CHILD | SS_CENTER,
		0, 320, GAME_WINDOW_WIDTH, 20, hWnd, NULL, hInstance, NULL);
	SetWindowText(hWnd_RoundTxt, "Runda [1]");

	//tworzenie pol gry
	int center = (GAME_WINDOW_WIDTH - 3 * (FIELD_SIZE + FIELD_PADDING)) / 2;
	int yOffset = 360;
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int x = center + i * (FIELD_SIZE + FIELD_PADDING);//wyliczamy srodek
			int y = yOffset + j * (FIELD_SIZE + FIELD_PADDING);
			HWND tmpButton = CreateWindowEx(0, "BUTTON", "", WS_CHILD,
				x,
				y,
				FIELD_SIZE,
				FIELD_SIZE,
				hWnd, NULL, hInstance, NULL
			);
			EnableWindow(tmpButton, FALSE);
			fields[i][j] = new GameField(
				tmpButton,
				{ x, y, x + FIELD_SIZE, y + FIELD_SIZE },
				x,
				y
			);
		}
	}


	hWnd_GameStateTxt = CreateWindowEx(0, "STATIC", NULL, WS_CHILD |
		SS_CENTER, 0, yOffset + 20 + 3 * FIELD_SIZE, GAME_WINDOW_WIDTH, 20, hWnd, NULL, hInstance, NULL);
	SetWindowText(hWnd_GameStateTxt, "Teraz rusza się Gracz #2");

	pathToPlayer1Symbol = GetPathToSymbol(symbolsFromData[player1Symbol]);
	pathToPlayer2Symbol = GetPathToSymbol(symbolsFromData[player2Symbol]);


	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

void NextGameSetupStep()
{
	++gameSetupStep;//nastepny krok ustawiania gry
	switch (gameSetupStep)
	{
	case 1:
		//pokazujemy te kontrolki ktore sa nam w tym momencie potrzebne
		ShowWindow(hWnd_SettingsTxt, SW_SHOW);
		ShowWindow(hWnd_PlayerTxt1, SW_SHOW);
		ShowWindow(hWnd_PlayerTxt2, SW_SHOW);

		ShowWindow(hWnd_ComboSymbols1, SW_SHOW);
		ShowWindow(hWnd_ComboSymbols2, SW_SHOW);

		ShowWindow(hWnd_NextColor1, SW_SHOW);
		ShowWindow(hWnd_NextColor2, SW_SHOW);
		ShowWindow(hWnd_PreviousColor2, SW_SHOW);

		ShowWindow(hWnd_CheckBoxPrimacy1, SW_SHOW);
		ShowWindow(hWnd_CheckBoxPrimacy2, SW_SHOW);

		ShowWindow(hWnd_StartButton, SW_SHOW);
		RedrawWindow(hWnd, new RECT{ 0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT }, 0, RDW_INVALIDATE | RDW_UPDATENOW);//musimy przerysowac okienko cale by pokazaly sie nam obrazki ktore sa zaczytane
		break;
	case 2:
		//pokazujemy te kontrolki ktore sa nam w tym momencie potrzebne
		ShowWindow(hWnd_RoundTxt, SW_SHOW);
		//a ukrywamy te zbedne
		ShowWindow(hWnd_SettingsTxt, SW_HIDE);
		ShowWindow(hWnd_StartButton, SW_HIDE);
		ShowWindow(hWnd_NextColor1, SW_HIDE);
		ShowWindow(hWnd_NextColor2, SW_HIDE);
		ShowWindow(hWnd_PreviousColor1, SW_HIDE);
		ShowWindow(hWnd_PreviousColor2, SW_HIDE);

		//dezakywujemy tez niektore zeby nie mozna bylo zmienic ustawionych wczesniej parametrow
		EnableWindow(hWnd_ComboSymbols1, FALSE);
		EnableWindow(hWnd_ComboSymbols2, FALSE);
		EnableWindow(hWnd_CheckBoxPrimacy1, FALSE);
		EnableWindow(hWnd_CheckBoxPrimacy2, FALSE);

		//zmieniamy napis na gorze okienka
		SetWindowText(hWnd_TitleTxt,
			(FirstToUpper(symbolsFromData[player1Symbol]) + " i " + FirstToUpper(symbolsFromData[player2Symbol])).c_str()
		);
		//czas pokazac przyciski pola tak zeby mozna bylo zaczac juz grac
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				ShowWindow(fields[i][j]->button, SW_SHOW);
			}
		}
		RedrawWindow(hWnd, new RECT{ 0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT }, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL colorSwitched = FALSE;
	switch (message)
	{
	case WM_COMMAND:
	{
		bool isButtonField = FALSE;
		//przede wszystkim musimy sprwadzic czy nie kliknelismy przycisku pola gry
		for (int i = 0; i < 3; ++i)
		{
			if (isButtonField == TRUE)
			{
				break;
			}
			for (int j = 0; j < 3; ++j)
			{
				if ((HWND)lParam == fields[i][j]->button)
				{
					isButtonField = TRUE;//jesli tak nie bedziemy juz sprawdzac pozostalych ifow dotyczacych innych przyciskow ktore sa nizej
					ShowWindow(fields[i][j]->button, SW_HIDE);//chowamy przycisk bo w jego miejsce narysujemy bitmape
					++usedFields;//zwiekszamy ilosc uzytych pol
					SetWindowText(hWnd_RoundTxt, ("Runda [" + to_string(usedFields + 1) + "]").c_str());//aktualizujemy napis mowiacy o tym ktora mamy runde
					if (player1Turn == TRUE)//jesli to byla runda gracza 1
					{
						player1Turn = FALSE;
						fields[i][j]->symbol = 1;//to wpisujemy do pola znak pierwszego gracza
						SetWindowText(hWnd_GameStateTxt, "Teraz rusza się Gracz #2");
					}
					else {//jesli rudna gracza 2
						player1Turn = TRUE;//przestawiamy kolejke
						fields[i][j]->symbol = -1;//to wpisujemy do pola znak drugiego gracza
						SetWindowText(hWnd_GameStateTxt, "Teraz rusza się Gracz #1");
					}
					RedrawWindow(hWnd, &fields[i][j]->imageRect, 0, RDW_INVALIDATE | RDW_UPDATENOW);//przerysowujemy okienko
																									//tylko w miejscu gdzie znajduje sie przycisk
					CheckForEndGame();//sprawdzamy czy moze ktos wygral
					if (playerVsPlayerMode == FALSE)//jesli gramy z komputerem to po prostu wywolujemy po naszym ruchu ruch komputera
					{
						ArticifalMove();
					}
					break;
				}
			}
		}
		if (isButtonField == FALSE)//jesli nie kliknieto zadnego przycisku pola gry to sprawdzmy inne
		{
			if ((HWND)lParam == hWnd_ModeButtonGVG)//przycisk trybu gracz kontra gracz
			{
				EnableWindow(hWnd_ModeButtonGVG, FALSE);//dezaktywujemy przyciski trybu gry
				EnableWindow(hWnd_ModeButtonGVK, FALSE);
				MessageBox(hWnd, "Wybrałeś tryb Gracz vs Gracz!", "Tryb gry", MB_ICONINFORMATION);//pokazujemy okienko
				playerVsPlayerMode = TRUE;//zmieniamy tryb na gracz vs gracz
				SetWindowText(hWnd_PlayerTxt1, "Gracz 1:");//zmieniamy teksty
				SetWindowText(hWnd_PlayerTxt2, "Gracz 2:");
				SetWindowText(hWnd_ModeTxt, "Tryb: Gracz vs Gracz");
				NextGameSetupStep();//przechodzimy do nastepnego stadium ustawien
			}
			else if ((HWND)lParam == hWnd_ModeButtonGVK)//przycisk trybu gracz kontra komputer analogiczny do 1
			{
				EnableWindow(hWnd_ModeButtonGVG, FALSE);
				EnableWindow(hWnd_ModeButtonGVK, FALSE);
				MessageBox(hWnd, "Wybrałeś tryb Gracz vs Komputer!", "Tryb gry", MB_ICONINFORMATION);
				playerVsPlayerMode = FALSE;
				SetWindowText(hWnd_PlayerTxt1, "Ty:");
				SetWindowText(hWnd_PlayerTxt2, "Komputer:");
				SetWindowText(hWnd_ModeTxt, "Tryb: Gracz vs Komputer");
				NextGameSetupStep();
			}
			else if ((HWND)lParam == hWnd_CheckBoxPrimacy1)//checkbox gracza 1 - zmiana pierwszenstwa na pierwszego gracza
			{
				SendMessage(hWnd_CheckBoxPrimacy2, BM_SETCHECK, BST_UNCHECKED, 0);//dezaktywujemy pierwszenstwo u gracza 2
			}
			else if ((HWND)lParam == hWnd_CheckBoxPrimacy2)//checkbox gracza 2 - zmiana pierwszenstwa na pierwszego gracza
			{
				SendMessage(hWnd_CheckBoxPrimacy1, BM_SETCHECK, BST_UNCHECKED, 0);//dezaktywujemy pierwszenstwo u gracza 1
			}
			else if (HWND(lParam) == hWnd_ComboSymbols1)//combo box z symbolami 1 gracza
			{
				switch (HIWORD(wParam))
				{
				case CBN_DROPDOWN:
					break;
				case BN_CLICKED:
					break;
				case CBN_SELCHANGE://jesli zmieniono wartosc
					int tmp = player1Symbol;//zapisujemy sobie na boku obecny index symbolu
					player1Symbol = SendMessage(hWnd_ComboSymbols1, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);//pobieramy index obiektu ktory wybral uzytykownik
					if (player1Color == player2Color && player2Symbol == player1Symbol)//jesli wybor symbolu i koloru dla gracza 1 i 2 jest taki sam to musimy temu zapobiec
					{
						SendMessage(hWnd_ComboSymbols1, CB_SETCURSEL, tmp, 0);//przywracamy stary index
						player1Symbol = tmp;
					}
					else {
						pathToPlayer1Symbol = GetPathToSymbol(symbolsFromData[player1Symbol]);//jesli nie jest taki sam to pobieramy sciezke do obrazka
						RedrawWindow(hWnd, new RECT{ 100, 180, 150, 230 }, 0, RDW_INVALIDATE | RDW_UPDATENOW);//przerysowujemy okienko w miejscu obrazka wyboru
					}
					break;
				}
			}
			else if (HWND(lParam) == hWnd_ComboSymbols2)//combo box z symbolami 2 gracza analogiczny do 1
			{
				switch (HIWORD(wParam))
				{
				case CBN_DROPDOWN:
					break;
				case BN_CLICKED:
					break;
				case CBN_SELCHANGE:
					int tmp = player2Symbol;
					player2Symbol = SendMessage(hWnd_ComboSymbols2, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
					if (player1Color == player2Color && player2Symbol == player1Symbol)
					{
						SendMessage(hWnd_ComboSymbols2, CB_SETCURSEL, tmp, 0);
						player2Symbol = tmp;
					}
					else {
						pathToPlayer2Symbol = GetPathToSymbol(symbolsFromData[player2Symbol]);
						RedrawWindow(hWnd, new RECT{ GAME_WINDOW_WIDTH - 150, 180, GAME_WINDOW_WIDTH - 100, 230 }, 0, RDW_INVALIDATE | RDW_UPDATENOW);
					}
					break;
				}
			}
			else if ((HWND)lParam == hWnd_NextColor1)//przycisk nastepnego koloru gracza 1
			{
				++player1Color;
				//jesli wybor symbolu i koloru dla gracza 1 i 2 jest taki sam to musimy temu zapobiec
				if (player1Color == player2Color && player2Symbol == player1Symbol)
				{
					++player1Color;//idziemy jeszcze jeden do przodu
					//jesli wyszlismy poza zakres to zerujemy wartosc indexu
					if (player1Color > Symbol_Colors.size() - 1)
					{
						player1Color = 0;
						ShowWindow(hWnd_PreviousColor1, SW_HIDE);
						ShowWindow(hWnd_NextColor1, SW_SHOW);
						colorSwitched = TRUE;//tak kolor sie zmienil
					}
				}
				//jesli doszlismy do konca chowamy przycisk nastepnego koloru zeby zapobiec wyjsciu poza zakres
				if (player1Color == Symbol_Colors.size() - 1)
				{
					ShowWindow(hWnd_NextColor1, SW_HIDE);
				}
				//sprwadzamy czy kolor sie zmienil
				if (colorSwitched == FALSE)
				{
					ShowWindow(hWnd_PreviousColor1, SW_SHOW);
				}
				RedrawWindow(hWnd, new RECT{ 100, 180, 150, 230 }, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}

			else if ((HWND)lParam == hWnd_PreviousColor1)//przycisk poprzedniego koloru gracza 1
			{
				--player1Color;
				//jesli wybor symbolu i koloru dla gracza 1 i 2 jest taki sam to musimy temu zapobiec
				if (player1Color == player2Color && player2Symbol == player1Symbol)
				{
					--player1Color;//idziemy jeszcze jeden do tylu
					if (player1Color < 0)//jesli wyszlismy poza zakres
					{
						player1Color = Symbol_Colors.size() - 1;//ustawiamy index na ostatni mozliwy
						ShowWindow(hWnd_PreviousColor1, SW_SHOW);
						ShowWindow(hWnd_NextColor1, SW_HIDE);
						colorSwitched = TRUE;//tak kolor sie zmienil
					}
				}
				//jesli doszlismy do konca chowamy przycisk poprzedniego koloru zeby zapobiec wyjsciu poza zakres
				if (player1Color == 0)
				{
					ShowWindow(hWnd_PreviousColor1, SW_HIDE);
				}
				if (colorSwitched == FALSE)
				{
					ShowWindow(hWnd_NextColor1, SW_SHOW);
				}
				RedrawWindow(hWnd, new RECT{ 100, 180, 150, 230 }, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if ((HWND)lParam == hWnd_NextColor2)//przycisk nastepnego koloru gracza 2 analogicznie do kodu wyzej
			{
				++player2Color;
				if (player1Color == player2Color && player2Symbol == player1Symbol)
				{
					++player2Color;
					if (player2Color > Symbol_Colors.size() - 1)
					{
						player2Color = 0;
						ShowWindow(hWnd_PreviousColor2, SW_HIDE);
						ShowWindow(hWnd_NextColor2, SW_SHOW);
						colorSwitched = TRUE;
					}
				}
				if (player2Color == Symbol_Colors.size() - 1)
				{
					ShowWindow(hWnd_NextColor2, SW_HIDE);
				}
				if (colorSwitched == FALSE)
				{
					ShowWindow(hWnd_PreviousColor2, SW_SHOW);
				}
				RedrawWindow(hWnd, new RECT{ GAME_WINDOW_WIDTH - 150, 180, GAME_WINDOW_WIDTH - 100, 230 }, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if ((HWND)lParam == hWnd_PreviousColor2)//przycisk poprzedniego koloru gracza 2 analogicznie do kodu wyzej
			{
				--player2Color;
				if (player1Color == player2Color && player2Symbol == player1Symbol)
				{
					--player2Color;
					if (player2Color < 0)
					{
						player2Color = Symbol_Colors.size() - 1;
						ShowWindow(hWnd_PreviousColor2, SW_SHOW);
						ShowWindow(hWnd_NextColor2, SW_HIDE);
						colorSwitched = TRUE;
					}
				}
				if (player2Color == 0)
				{
					ShowWindow(hWnd_PreviousColor2, SW_HIDE);
				}
				if (colorSwitched == FALSE)
				{
					ShowWindow(hWnd_NextColor2, SW_SHOW);
				}
				RedrawWindow(hWnd, new RECT{ GAME_WINDOW_WIDTH - 150, 180, GAME_WINDOW_WIDTH - 100, 230 }, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else if ((HWND)lParam == hWnd_StartButton)//przycisk rozpoczecia gry
			{
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						EnableWindow(fields[i][j]->button, TRUE);//aktywujemy przyciski pola gry
					}
				}
				if (SendMessage(hWnd_CheckBoxPrimacy1, BM_GETCHECK, 0, 0) == BST_UNCHECKED)//sprawdzamy czy chcecbox pierwszenstwa gracza 1 jest odznaczony
				{
					player1Turn = FALSE;//jesli tak to teraz tura gracza 2
				}
				if (!playerVsPlayerMode)//jesli jest to tryb gracz vs gracz
				{
					ShowWindow(hWnd_GameStateTxt, SW_HIDE);
					if (player1Turn == FALSE)//jesli to nie jest tura gracza
					{
						ArticifalMove();//to wywolujemy ruch komputera
					}
				}
				else {
					if (player1Turn == TRUE)//w zaleznosci od tego czyja to tura to pokazujemy odpowiedni tekst
					{
						SetWindowText(hWnd_GameStateTxt, "Teraz rusza się Gracz #1");
					}
					else {
						SetWindowText(hWnd_GameStateTxt, "Teraz rusza się Gracz #2");
					}
				}
				NextGameSetupStep();//przechodzimy do nastepnego kroku w ustawianiu
			}
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		//sprawdzamy czy juz wybrano tryb gry
		if (gameSetupStep > 0)
		{
			HBITMAP hbmOld;
			//wczytanie bitmap
			//raz symbolu pierwszego gracza
			HBITMAP hbmpSymbol1 = (HBITMAP)LoadImage(NULL, pathToPlayer1Symbol.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			//raz symbolu drugiego gracza
			HBITMAP hbmpSymbol2 = (HBITMAP)LoadImage(NULL, pathToPlayer2Symbol.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			HDC hdcForBitmap = CreateCompatibleDC(hdc);
			BITMAP bmInfo1, bmInfo2;

			//pobieranie wielkosci bitmapy zebysmy wiedzieli jak duza ona jest by poprawnie ja narysowac zgodnie z proporcjami
			GetObject(hbmpSymbol1, sizeof(bmInfo1), &bmInfo1);
			RECT player1_rect = { 100,      180,            150,            230 };
			//                     left Współrzędna x lewego górnego rogu prostokąta.
			//                              top Współrzędna y lewego górnego rogu prostokąta.
			//                                              right   Współrzędna x prawego dolnego rogu prostokąta.
			//                                                              bottom  Współrzędna y prawego dolnego rogu prostokąta
			RECT player2_rect = { GAME_WINDOW_WIDTH - 150, 180, GAME_WINDOW_WIDTH - 100, 230 };

		

			HBRUSH Player1Brush = CreateSolidBrush(Symbol_Colors[player1Color]);
			HBRUSH Player2Brush = CreateSolidBrush(Symbol_Colors[player2Color]);

			//rysowanie kwadratow z kolorem
			FillRect(hdc, &player1_rect, Player1Brush);
			FillRect(hdc, &player2_rect, Player2Brush);

			hbmOld = (HBITMAP)SelectObject(hdcForBitmap, hbmpSymbol1);

			//rysowanie bitmapy gracza 1 w ustawieniach
			BitBlt(hdc, 100, 180, FIELD_SIZE, FIELD_SIZE, hdcForBitmap, 0, 0, SRCAND);
			SelectObject(hdcForBitmap, hbmpSymbol1);

			hbmOld = (HBITMAP)SelectObject(hdcForBitmap, hbmpSymbol2);
			//rysowanie bitmapy gracza 2 w ustawieniach
			BitBlt(hdc, GAME_WINDOW_WIDTH - 150, 180, FIELD_SIZE, FIELD_SIZE, hdcForBitmap, 0, 0, SRCAND);
			SelectObject(hdcForBitmap, hbmpSymbol2);

			//sprawdzamy czy czy gra juz trwa
			if (gameSetupStep > 1)
			{
				//tutaj rysowane sa obrazki na polach gry
				HBRUSH FieldsBrush = CreateSolidBrush(0x333333);
				FillRect(hdc, new RECT
					{
						fields[0][0]->x,
						fields[0][0]->y,
						fields[0][0]->x + 3 * FIELD_SIZE + 2 * FIELD_PADDING,
						fields[0][0]->y + 3 * FIELD_SIZE + 2 * FIELD_PADDING
					}, FieldsBrush);
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						//jesli to pole zajal 1 gracz to narysuje sie tu jego kwadrat z kolorem a potem bitmapa a jesli drugi to 2
						if (fields[i][j]->symbol == 1)
						{
							FillRect(hdc, &fields[i][j]->imageRect, Player1Brush);
							hbmOld = (HBITMAP)SelectObject(hdcForBitmap, hbmpSymbol1);
							BitBlt(hdc, fields[i][j]->x, fields[i][j]->y, FIELD_SIZE, FIELD_SIZE, hdcForBitmap, 0, 0, SRCAND);
							SelectObject(hdcForBitmap, hbmpSymbol1);
						}
						else if (fields[i][j]->symbol == -1)
						{
							FillRect(hdc, &fields[i][j]->imageRect, Player2Brush);
							hbmOld = (HBITMAP)SelectObject(hdcForBitmap, hbmpSymbol2);
							BitBlt(hdc, fields[i][j]->x, fields[i][j]->y, FIELD_SIZE, FIELD_SIZE, hdcForBitmap, 0, 0, SRCAND);
							SelectObject(hdcForBitmap, hbmpSymbol2);
						}
					}
				}
			}

			//zwolnienie
			ReleaseDC(hWnd, hdc);
			DeleteObject(hbmpSymbol1);
			DeleteObject(hbmpSymbol2);
			SelectObject(hdcForBitmap, hbmOld);
			DeleteDC(hdcForBitmap);
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//ruch komputera
void ArticifalMove()
{
	if (gameResult != 0)//jesli gra sie skonczyla (ma inna wartosc niz 0) to nie robmy nic
	{
		return;
	}
	srand(time(NULL));//inicjalizacja losowosci
	int rx, ry;
	BOOL fieldReserved = TRUE;
	while (fieldReserved == TRUE)//dopoki nie znajdziemy wolnego pola
	{
		rx = rand() % 3;//losujemy x
		ry = rand() % 3;//losujemy y
		fieldReserved = FALSE;//resetujemy czy znalezlismy zajete pole
		if (fields[rx][ry]->symbol == 0)//jesli pole na pozycji rx,ry jest wolne to
		{
			++usedFields;//zwiekszamy liczbe zuzytych pol
			fields[rx][ry]->symbol = -1;//do wylosowanego pola przypisujemy symbol drugiego gracza (komputer zawsze jest drugim graczem)
			RedrawWindow(hWnd, &fields[rx][ry]->imageRect, 0, RDW_INVALIDATE | RDW_UPDATENOW);//przerysowujemy
			SetWindowText(hWnd_RoundTxt, ("Runda [" + to_string(usedFields + 1) + "]").c_str());//zmieniamy tekst rundy
			ShowWindow(fields[rx][ry]->button, SW_HIDE);//chowamy przycisk pola ktory wybral komputer
		}
		else {
			fieldReserved = TRUE;//jesli nie to musimy od nowa szukac
		}
	}
	player1Turn = TRUE;//teraz ruch gracza
	CheckForEndGame();//sprawdzamy czy gra sie nie skonczyla
}

//warunki zwyciestwa, przegranej, remisu
void CheckForEndGame()
{
	//Gracz1
	if (
		//W poziomie
		(fields[0][0]->symbol == 1 && fields[0][1]->symbol == 1 && fields[0][2]->symbol == 1) ||
		(fields[1][0]->symbol == 1 && fields[1][1]->symbol == 1 && fields[1][2]->symbol == 1) ||
		(fields[2][0]->symbol == 1 && fields[2][1]->symbol == 1 && fields[2][2]->symbol == 1) ||
		//W pionie
		(fields[0][0]->symbol == 1 && fields[1][0]->symbol == 1 && fields[2][0]->symbol == 1) ||
		(fields[0][1]->symbol == 1 && fields[1][1]->symbol == 1 && fields[2][1]->symbol == 1) ||
		(fields[0][2]->symbol == 1 && fields[1][2]->symbol == 1 && fields[2][2]->symbol == 1) ||
		//Na ukos
		(fields[0][0]->symbol == 1 && fields[1][1]->symbol == 1 && fields[2][2]->symbol == 1) ||
		(fields[0][2]->symbol == 1 && fields[1][1]->symbol == 1 && fields[2][0]->symbol == 1)
		)
	{
		gameResult = 1;//wygral gracz 1
	}
	//Gracz2 / Komputer
	else if
		(
			//W poziomie
		(fields[0][0]->symbol == -1 && fields[0][1]->symbol == -1 && fields[0][2]->symbol == -1) ||
			(fields[1][0]->symbol == -1 && fields[1][1]->symbol == -1 && fields[1][2]->symbol == -1) ||
			(fields[2][0]->symbol == -1 && fields[2][1]->symbol == -1 && fields[2][2]->symbol == -1) ||
			//W pionie                                                                           
			(fields[0][0]->symbol == -1 && fields[1][0]->symbol == -1 && fields[2][0]->symbol == -1) ||
			(fields[0][1]->symbol == -1 && fields[1][1]->symbol == -1 && fields[2][1]->symbol == -1) ||
			(fields[0][2]->symbol == -1 && fields[1][2]->symbol == -1 && fields[2][2]->symbol == -1) ||
			//Na ukos                                                                            
			(fields[0][0]->symbol == -1 && fields[1][1]->symbol == -1 && fields[2][2]->symbol == -1) ||
			(fields[0][2]->symbol == -1 && fields[1][1]->symbol == -1 && fields[2][0]->symbol == -1)
			)
	{
		gameResult = -1;//wygral gracza 2 / komputer
	}
	else if (usedFields >= 9) {//jesli ilosc zuzytych pol osiagnela ilosc mozliwych pol (plansza ma 3x3 czyli 9 pol)
		gameResult = 2;
	}

	if (gameResult != 0)//jesli gra skonczyla sie to
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				EnableWindow(fields[i][j]->button, FALSE);//dezaktywujemy pola zeby nie dalo sie juz nic zrobic
			}
		}
		if (gameResult == 1)//jesli wygral gracz 1
		{
			if (playerVsPlayerMode == TRUE)// gracz vs gracz
			{
				string win1 = ("Gracz #1 [" + FirstToUpper(symbolsFromData[player1Symbol]) + "] zwyciężył!");
				MessageBox(hWnd, win1.c_str(), "Gracz #1 i jego Zapach Zwycięstwa!", MB_ICONINFORMATION);
				SetWindowText(hWnd_RoundTxt, win1.c_str());
			}
			else {//jesli to byla gra z komputerem
				MessageBox(hWnd, "Wygrałeś z Komputerem!", MB_ICONINFORMATION);
				SetWindowText(hWnd_RoundTxt, "Wygrałeś! Pokonałeś Komputer!");
			}
		}
		else if (gameResult == -1)// jesli wygral gracz 2
		{
			if (playerVsPlayerMode == TRUE)//i byl to tryb gracz vs gracz
			{
				string win2 = ("Gracz #2 [" + FirstToUpper(symbolsFromData[player2Symbol]) + "] zwyciężył!");
				MessageBox(hWnd, win2.c_str(), "Gracz #2 i jego Zapach Zwycięstwa!", MB_ICONINFORMATION);
				SetWindowText(hWnd_RoundTxt, win2.c_str());
			}
			else {//jesli to byla gra z komputerem (komputer zawsze jest graczem 2 wiec to oznacza ze on tu wygral
				MessageBox(hWnd, "Przegrałeś!" MB_ICONWARNING);
				SetWindowText(hWnd_RoundTxt, "Przegrałeś! Komputer zwyciężył!");
			}
		}
		else if (gameResult == 2)//jesli to byl remis
		{
			MessageBox(hWnd, "Tej rozgrywki nikt nie wygrał!", "Wyrównane Siły!", MB_ICONWARNING);
			SetWindowText(hWnd_RoundTxt, "Tej rozgrywki nikt nie wygrał! Wyrównane Siły!");
		}
	}
}