
import yfinance as yf


def get_stock_data(symbol):
    symbol = str(symbol).strip().upper()

    if not symbol:
        return None

    if not symbol.endswith((".NS", ".BO")):
        symbol += ".NS"

    try:
        stock = yf.Ticker(symbol)

        data = stock.history(
            period="3mo",
            interval="1d",
            auto_adjust=True
        )

        if data.empty:
            return None

        data = data.tail(30).reset_index()

        results = []

        for _, row in data.iterrows():
            results.append({
                "date": str(row["Date"].date()),
                "open": round(float(row["Open"]), 2),
                "high": round(float(row["High"]), 2),
                "low": round(float(row["Low"]), 2),
                "close": round(float(row["Close"]), 2),
                "volume": int(row["Volume"])
            })

        return results if results else None

    except Exception as error:
        raise RuntimeError(
            f"Unable to retrieve stock data: {error}"
        )
