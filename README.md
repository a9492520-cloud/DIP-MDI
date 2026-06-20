# DIP-MDI — 數位影像處理之金屬字元辨識

Visual Studio C# WinForms + C++ DLL + Python PaddleOCR 混合架構的影像處理與金屬字元（Lot Number）辨識系統。

## 專案結構

```
DIP-MDI/
├── DIP/                     # C# WinForms 主程式 (.NET Framework 4.8)
│   ├── DIPSample.cs         # 主視窗：金屬字元辨識（呼叫 Python 子程序）
│   ├── Form1~Form9.cs       # 各種影像處理功能表單
│   └── MSForm.cs            # MDI 子視窗（顯示圖片用）
│
├── DIP_proc/                # C++ DLL 原生影像處理函式庫
│   ├── dip_proc.cpp         # DLL 匯出介面
│   ├── lot_number.h         # C++ 傳統金屬字元辨識（樣板比對）
│   ├── metal_ocr.py         # ★ Python PaddleOCR 辨識腳本
│   ├── filters.h            # 濾波器
│   ├── otsu.h               # 大津二值化
│   ├── histogram.h          # 直方圖
│   ├── gray_bitslice.h      # 灰階位元切片
│   ├── resize_median.h      # 縮放與中值濾波
│   ├── rotation.h           # 旋轉
│   ├── line_detection.h     # 直線偵測
│   └── circle_detection.h   # 圓形偵測
│
├── Cal/                     # C++ 校正用 DLL
├── 圖/                      # 測試影像
│   ├── lot_number_01~12.bmp # 12 張金屬字元測試圖
│   └── ...其他測試圖
├── DIP VerB.sln             # Visual Studio 解決方案檔
└── .gitignore
```

## 金屬字元辨識（Lot Number Recognition）

### 輸入/輸出格式

- **輸入**：金屬零件表面影像（刻印或點陣的 6 碼英數字 Lot Number）
- **輸出**：格式化的 6 碼字串，前 5 碼為數字、第 6 碼為英文字母（如 `11240B`、`09240A`）

### 兩種實作方案比較

#### 方案 A：C++ 傳統樣板比對（已棄用）

位於 `lot_number.h` 的 `recognizeLotNumber()`，管線如下：

```
ROI截取(20%,40%,65%,40%)
  → 3x放大(INTER_LINEAR)
  → 對比增強(Unsharp Mask)
  → Gaussian 5×5平滑
  → 適應性二值化(門檻值C)
  → 文字列偵測(水平投影)
  → 型態學Close(半徑4)
  → 垂直投影字元切割 → ★區段數(segCount)★
  → 每個區段Max-pool正規化12×16
  → NCC樣板比對(36組: 0-9, A-Z)
  → 輸出6碼字串
```

**區段數 (segCount)**：垂直投影切割找到的連續區塊數量。理想的 6 碼字元應切出 6 段，但常因：

| 情況 | 原因 |
|------|------|
| segCount=6 | 字元間距均勻、切割理想 |
| segCount=7 | 某字元（如 0、8、A、B）中間有凹陷，垂直投影低於門檻而被切成兩段 |
| segCount=5↓ | Close 半徑過大導致相鄰字元黏在一起 |
| segCount=0/ERROR | 二值化極性錯誤（字元暗→變背景）、門檻值不合適 |

**為什麼 C++ 辨識失敗？**
1. **極性反轉問題**：`f > mean - C ? 255 : 0` 假設字元是亮點，但金屬刻印字元實際上是**暗點**（陰影），導致二值化後字元與背景互換
2. **3x + Max-pool 使字元過胖**：正規化後白點占比 60-80%，而樣板僅 20-40%，NCC 比對分數極低（<0.40）
3. **對光線變化敏感**：金屬反光造成二值化結果不穩定

#### 方案 B：Python PaddleOCR（現行方案）

位於 `DIP_proc/metal_ocr.py`，由 C# 以子程序方式呼叫：

```
python metal_ocr.py <image_path>
```

管線如下：

```
ROI截取(20%,40%,65%,40%)
  → 3x放大(INTER_CUBIC)
  → CLAHE對比度限制自適應直方圖均衡化
  → GaussianBlur 5×5
  → PaddleOCR(en模式, 無cls, 無GPU)
  → format_lot_number()格式化輸出
```

**C# 呼叫方式**（`DIPSample.cs`）：
```csharp
// 儲存 Bitmap 為暫存 BMP
src.Save(tempImg, ImageFormat.Bmp);
// 啟動 Python 子程序
Process.Start("python", $"\"{scriptPath}\" \"{tempImg}\"");
// 讀取 stdout 辨識結果
result = p.StandardOutput.ReadToEnd().Trim();
```

**格式化函數 `format_lot_number()`**：
將 PaddleOCR 原始輸出轉換為標準格式：
- 前 5 碼：限制為數字（O→0, I→1, B→8, S→5, Z→2 等）
- 第 6 碼：限制為字母（8→B, 0→O, 1→I, 5→S 等）

### 測試結果

| 測試圖 | C++ 傳統辨識結果 | PaddleOCR 結果 |
|--------|-----------------|----------------|
| 01-03 | 00001J / 00401V（亂碼） | **11240B** |
| 04-06 | 00401V / 亂碼 | **09240A** |
| 07-09 | 00301V / 亂碼 | **09740C** |
| 10-12 | 00501V / 亂碼 | **12040A** |

PaddleOCR 12 張全數正確，證明了深度學習方案對金屬凹凸字元的強健性。

## 環境需求

### 建置需求
- Visual Studio 2022（含「使用 C++ 的桌面開發」與「.NET 桌面開發」工作負載）
- Windows SDK

### 執行需求
- Python 3.12+
- Python 套件：`pip install opencv-python numpy paddlepaddle paddleocr`

### 建置步驟
1. 以 Visual Studio 開啟 `DIP VerB.sln`
2. 建置 `DIP_proc`（C++ DLL，產生 DIP_proc.dll）
3. 建置 `DIP`（C# WinForms，自動複製 DIP_proc.dll 到輸出目錄）
4. 執行 `DIP.exe` 並開啟測試圖 → 點選選單「金屬字元辨識」

## 備註

- C++ DLL (`debug_lot_number`) 仍保留作為除錯用途，顯示傳統 CV 管線的前處理二值圖與區段數，與 Python PaddleOCR 結果做對比
- Python 腳本放在 `DIP_proc/` 而非根目錄，與相關 C++ 源碼一起管理
