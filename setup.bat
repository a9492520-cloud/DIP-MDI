@echo off
chcp 65001 >nul
title DIP-MDI 環境設定工具

echo ============================================
echo   DIP-MDI 金屬字元偵測 - 環境設定
echo ============================================
echo.

REM 檢查 Python
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [錯誤] 找不到 Python！請先安裝 Python 3.8 ~ 3.12
    echo 下載網址: https://www.python.org/downloads/
    echo 安裝時請記得勾選 "Add Python to PATH"
    pause
    exit /b 1
)

echo [1/3] 建立虛擬環境...
if exist "portable_python\Scripts\python.exe" (
    echo   虛擬環境已存在，跳過此步驟
) else (
    python -m venv portable_python
    if %errorlevel% neq 0 (
        echo [錯誤] 建立虛擬環境失敗！
        pause
        exit /b 1
    )
    echo   虛擬環境建立完成
)

echo [2/3] 更新 pip...
call portable_python\Scripts\python.exe -m pip install --upgrade pip -q
echo   完成

echo [3/3] 安裝必要套件 (opencv-python, numpy)...
call portable_python\Scripts\python.exe -m pip install -r requirements.txt -q
if %errorlevel% neq 0 (
    echo [錯誤] 安裝套件失敗！
    pause
    exit /b 1
)
echo   完成

echo.
echo ============================================
echo   環境設定完成！現在可以用 Visual Studio
echo   開啟 DIP VerB.sln 來編譯與執行。
echo ============================================
echo.
pause
