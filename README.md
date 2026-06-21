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

## 邊緣缺陷檢測（Edge Defect Detection）

位於 `edge_defect_detection.h`，使用形態學與連通元件分析偵測金屬邊緣的崩角、毛刺等缺陷。

完整流程如下：

```
輸入灰階影像
  → Otsu 二值化 (otsuThreshold)
  → 圓形閉合運算 (morphClose, radius=120)
  → 影像相減提取缺陷 (closed - binary)
  → 連通元件標記 (connectedComponents)
  → 缺陷特徵篩選 (面積、長寬比、高度)
  → 繪製外框標記缺陷
```

---

### 1. 自適應二值化（otsuThreshold）

```cpp
int threshold;
otsuThreshold(f, w, h, threshold);
for (int i = 0; i < total; i++)
    binary[i] = (f[i] >= threshold) ? 255 : 0;
```

**說明**：自動根據整張圖的灰階分布統計出一條「最佳黃金分割線」（即 threshold），並將原圖 `f` 轉換成非黑即白的二值化影像 `binary`。閾值的選取是透過 Otsu 演算法，最大化類間方差，使背景與前景的分離達到最佳。

---

### 2. 圓形核心的閉合運算（morphClose）

```cpp
static void morphClose(int* f, int w, int h, int* g, int radius)
{
    int* temp = new int[w * h];
    dilate(f, w, h, temp, radius);   // 先膨脹
    erode(temp, w, h, g, radius);    // 後侵蝕
    delete[] temp;
}
```

**核心產生（getEllipseOffsets）**：根據設定的半徑（`radius = 120`），利用圓形方程式 `i² + j² = r²` 計算出圓形遮罩的相對座標偏移量。

**膨脹（dilate）**：取核心範圍內的最大值（maxVal）。這會擴大白色區域，將邊緣的小凹陷或毛刺「填滿、推平」。

**侵蝕（erode）**：取核心範圍內的最小值（minVal）。將剛剛膨脹過頭的白色主體縮回原本的大小，但已經被抹平的毛刺不會再回來。

**閉合（morphClose）**：就是連續執行「先膨脹、後侵蝕」，得到一個平滑邊緣的影像 `closed`。

---

### 3. 影像相減提取缺陷（closed - binary）

```cpp
defect[i] = closed[i] - binary[i];
if (defect[i] < 0) defect[i] = 0;
```

**說明**：把「完美平滑的影像（`closed`）」減去「原本有毛刺的影像（`binary`）」。原本一樣的地方相減會變為 0（黑色），而只有原圖中那個突出的缺陷毛刺會被留下來，存入 `defect` 影像。

---

### 4. 連通元件標記法（connectedComponents）

```cpp
static int connectedComponents(int* f, int w, int h, int* labels) { ... }
```

**Union-Find 集合架構**：程式宣告了 `UFNode`，利用並查集（Disjoint-set / Union-Find）演算法來記錄像素間的連通關係。

**雙掃描標記**：逐一掃描像素，如果當前像素與其「左方」或「上方」的像素相連，就把它們歸為同一個標籤（Label）；若左右標籤不同則進行合併（`unite`）。這能將畫面上每一塊獨立的白色雜點或毛刺編號。

最終回傳 `compCount`：即總共找到了多少塊獨立的連通區域。

---

### 5. 缺陷特徵篩選與繪製外框（Filtering & Bounding Box）

```cpp
if (comps[i].area >= 20 && comps[i].area <= 2000
    && aspectRatio >= 1.2 && ch >= 8)
```

篩選條件如下：

| 條件 | 範圍 | 說明 |
|------|------|------|
| 面積（area） | 20 ~ 2000 像素 | 太小是雜訊，太大可能整張圖壞了 |
| 長寬比（aspectRatio） | ≥ 1.2 | 代表它是一根細長、突出的刺（高度 > 寬度） |
| 高度（ch） | ≥ 8 像素 | 排除過小的非缺陷區塊 |

符合條件的缺陷會在外圍繪製紅色矩形外框，標記在結果影像上。

---

## 備註

- C++ DLL (`debug_lot_number`) 仍保留作為除錯用途，顯示傳統 CV 管線的前處理二值圖與區段數，與 Python PaddleOCR 結果做對比
- Python 腳本放在 `DIP_proc/` 而非根目錄，與相關 C++ 源碼一起管理
