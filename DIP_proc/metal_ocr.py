import cv2
import numpy as np
import sys
import os

def _imread_unicode(path):
    try:
        with open(path, 'rb') as f:
            data = f.read()
        img = cv2.imdecode(np.frombuffer(data, np.uint8), cv2.IMREAD_COLOR)
        return img
    except:
        return None

def get_lot_roi(img):
    h, w = img.shape[:2]
    roi_x = int(w * 0.20)
    roi_y = int(h * 0.40)
    roi_w = int(w * 0.65)
    roi_h = int(h * 0.40)
    if roi_x + roi_w > w: roi_w = w - roi_x
    if roi_y + roi_h > h: roi_h = h - roi_y
    if roi_w < 1: roi_w = 1
    if roi_h < 1: roi_h = 1
    roi = img[roi_y:roi_y + roi_h, roi_x:roi_x + roi_w]
    return roi

def format_lot_number(raw_chars):
    digit_map = {'O': '0', 'Q': '0', 'D': '0', 'I': '1', 'L': '1', 'Z': '2', 'S': '5', 'G': '6', 'B': '8'}
    letter_map = {'8': 'B', '0': 'O', '1': 'I', '5': 'S', '2': 'Z', '6': 'G'}
    filtered = []
    for c in raw_chars:
        if c and c.isalnum():
            filtered.append(c.upper())
    result = []
    for i in range(6):
        if i < len(filtered):
            c = filtered[i]
        else:
            c = '0' if i < 5 else 'A'
        if i < 5:
            c = digit_map.get(c, c)
            if not c.isdigit(): c = '0'
        else:
            c = letter_map.get(c, c)
            if not c.isalpha(): c = 'A'
        result.append(c)
    return ''.join(result[:6])

def extract_text_from_ocr(result):
    texts = []
    for line_group in result:
        for item in line_group:
            box, (text, score) = item
            texts.append(text.strip())
    combined = ''.join(texts)
    return combined

def recognize(image_path):
    img = _imread_unicode(image_path)
    if img is None:
        return "ERROR"

    roi = get_lot_roi(img)

    roi_big = cv2.resize(roi, None, fx=3, fy=3, interpolation=cv2.INTER_CUBIC)

    lab = cv2.cvtColor(roi_big, cv2.COLOR_BGR2LAB)
    l, a, b = cv2.split(lab)
    clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8, 8))
    l = clahe.apply(l)
    enhanced = cv2.merge([l, a, b])
    enhanced = cv2.cvtColor(enhanced, cv2.COLOR_LAB2BGR)

    smoothed = cv2.GaussianBlur(enhanced, (5, 5), 0)

    try:
        from paddleocr import PaddleOCR
        ocr = PaddleOCR(use_angle_cls=False, lang='en', show_log=False, use_gpu=False)
        result = ocr.ocr(smoothed, cls=False)
    except Exception as e:
        return f"ERROR:OCR:{e}"

    raw_text = extract_text_from_ocr(result)

    lot_number = format_lot_number(raw_text)
    return lot_number

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("ERROR: Usage: python metal_ocr.py <image_path>")
        sys.exit(1)
    image_path = sys.argv[1]
    if not os.path.exists(image_path):
        print(f"ERROR: File not found: {image_path}")
        sys.exit(1)
    result = recognize(image_path)
    print(result)
