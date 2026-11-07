# CandleSTICKS
### Candlestick Pattern Recognition System

CandleSTICKS is a web-based stock market analysis application that helps users explore stock price movements and identify candlestick patterns.

The project uses C++ for pattern recognition, Python and Flask for backend processing, and HTML, CSS and JavaScript for the frontend.

## Features

- **Automatic Stock Analysis:** Search for Indian stocks using their symbols and fetch the latest 30 daily candles.
- **Manual Candle Analysis:** Enter your own Open, High, Low and Close values to analyze candles.
- **Pattern Recognition:** Detect single-candle, two-candle and three-candle patterns.
- **Pattern Insights:** Display detected pattern names, bullish/bearish/neutral signals and descriptions.
- **Multiple Candle Input:** Add multiple candles to explore different pattern formations.

## Technologies Used

- C++17
- Python
- Flask
- yfinance
- HTML, CSS and JavaScript

## How It Works

Users can either search for a stock or enter their own candlestick values. For stock analysis, historical market data is retrieved using yfinance. The candle data is then passed to a C++ program that identifies recognized patterns. The results are sent back through the Flask backend and displayed on the website.

## How to Run

**1. Install dependencies**

```bash
py -m pip install -r backend/requirements.txt
```

**2. Compile the C++ program**

Open the terminal inside the `backend` folder:

```bash
g++ -std=c++17 candlestickpatternrecognition.cpp -o Candlestick.exe
```

**3. Start the application**

```bash
py app.py
```

**4. Open in your browser**

http://127.0.0.1:5000/

---

*Built with curiosity, creativity and code.*