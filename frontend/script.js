
"use strict";

function renderPatterns(candles, output, target) {
    target.replaceChildren();

    const heading = document.createElement("h4");
    heading.textContent = "Detected Candlestick Patterns";
    target.appendChild(heading);

    const detected = output?.patterns || [];

    if (!detected.length) {
        const message = document.createElement("p");
        message.textContent = "No patterns detected in the entered candles.";
        target.appendChild(message);
        return;
    }

    detected.forEach(pattern => {
        const start = candles[pattern.startIndex];
        const end = candles[pattern.endIndex];

        if (!start || !end) return;

        const card = document.createElement("div");
        card.style.cssText =
            "background:#080d18;padding:18px;margin:15px 0;border-radius:10px;border:1px solid #263b54;";

        const title = document.createElement("h4");
        title.textContent = pattern.name;
        title.style.color = "#00d4aa";

        const direction = document.createElement("p");
        direction.textContent = `Signal: ${pattern.direction}`;

        const date = document.createElement("p");
        date.textContent = start.date === end.date
            ? `Candle: ${end.date || `Candle ${pattern.endIndex + 1}`}`
            : `Candles: ${start.date || `Candle ${pattern.startIndex + 1}`} to ${end.date || `Candle ${pattern.endIndex + 1}`}`;

        const prices = document.createElement("p");
        prices.textContent =
            `Open: ${end.open} | High: ${end.high} | Low: ${end.low} | Close: ${end.close}`;

        const description = document.createElement("p");
        description.textContent = pattern.description;
        description.style.marginTop = "10px";

        card.append(title, direction, date, prices, description);
        target.appendChild(card);
    });
}


async function analyzeStock() {
    const symbolInput = document.getElementById("symbol");
    const button = document.getElementById("analyzeBtn");
    const loading = document.getElementById("loading");
    const error = document.getElementById("error");
    const results = document.getElementById("results");
    const stockInfo = document.getElementById("stockInfo");
    const patterns = document.getElementById("patterns");

    const symbol = symbolInput.value.trim().toUpperCase();

    if (!symbol) {
        error.textContent = "Enter a stock symbol first.";
        results.style.display = "none";
        return;
    }

    error.textContent = "";
    results.style.display = "none";
    stockInfo.replaceChildren();
    patterns.replaceChildren();
    loading.style.display = "block";
    button.disabled = true;

    try {
        const response = await fetch("/analyse", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ symbol })
        });

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Stock analysis failed.");
        }

        stockInfo.textContent =
            `${data.symbol} | ${data.candles.length} candles analyzed`;

        renderPatterns(data.candles, data.patterns, patterns);
        results.style.display = "block";

    } catch (err) {
        error.textContent = err.message || "Unable to analyze stock.";
    } finally {
        loading.style.display = "none";
        button.disabled = false;
    }
}


function addCandleRow() {
    const container = document.getElementById("candleInputs");
    const candleNumber = container.querySelectorAll(".candle-input-row").length + 1;

    const row = document.createElement("div");
    row.className = "candle-input-row";

    row.innerHTML = `
        <h4>Candle ${candleNumber}</h4>
        <div class="manual-grid">
            <input type="number" step="any" class="open-price" placeholder="Open" required>
            <input type="number" step="any" class="high-price" placeholder="High" required>
            <input type="number" step="any" class="low-price" placeholder="Low" required>
            <input type="number" step="any" class="close-price" placeholder="Close" required>
            <input type="number" step="any" class="volume-price" placeholder="Volume (optional)" value="0">
        </div>
        <button type="button" class="remove-candle-btn">REMOVE CANDLE</button>
    `;

    row.querySelector(".remove-candle-btn").addEventListener("click", () => {
        row.remove();
        updateCandleNumbers();
    });

    container.appendChild(row);
}


function updateCandleNumbers() {
    const rows = document.querySelectorAll(".candle-input-row");

    rows.forEach((row, index) => {
        row.querySelector("h4").textContent = `Candle ${index + 1}`;
    });
}


async function analyzeManualCandles() {
    const button = document.getElementById("analyzeManualBtn");
    const container = document.getElementById("candleInputs");

    const error = document.getElementById("manualError");
    const loading = document.getElementById("manualLoading");
    const results = document.getElementById("manualResults");
    const stockInfo = document.getElementById("manualStockInfo");
    const patterns = document.getElementById("manualPatterns");

    error.textContent = "";
    results.style.display = "none";
    stockInfo.textContent = "";
    patterns.replaceChildren();

    const rows = container.querySelectorAll(".candle-input-row");

    if (!rows.length) {
        error.textContent = "Add at least one candle.";
        return;
    }

    const candles = [];

    for (let index = 0; index < rows.length; index++) {
        const row = rows[index];

        const open = Number(row.querySelector(".open-price").value);
        const high = Number(row.querySelector(".high-price").value);
        const low = Number(row.querySelector(".low-price").value);
        const close = Number(row.querySelector(".close-price").value);

        const volumeInput = row.querySelector(".volume-price").value.trim();
        const volume = volumeInput === "" ? 0 : Number(volumeInput);

        if (
            row.querySelector(".open-price").value === "" ||
            row.querySelector(".high-price").value === "" ||
            row.querySelector(".low-price").value === "" ||
            row.querySelector(".close-price").value === ""
        ) {
            error.textContent = `Fill in all four prices for Candle ${index + 1}.`;
            return;
        }

        if (
            ![open, high, low, close, volume].every(Number.isFinite) ||
            open <= 0 || high <= 0 || low <= 0 || close <= 0 || volume < 0
        ) {
            error.textContent = `Enter valid positive prices and a non-negative volume for Candle ${index + 1}.`;
            return;
        }

        if (
            high < Math.max(open, close) ||
            low > Math.min(open, close) ||
            high < low
        ) {
            error.textContent = `Invalid OHLC values for Candle ${index + 1}. Check High and Low.`;
            return;
        }

        candles.push({
            date: `Candle ${index + 1}`,
            open,
            high,
            low,
            close,
            volume
        });
    }

    button.disabled = true;
    loading.style.display = "block";

    try {
        const response = await fetch("/analyse-candles", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ candles })
        });

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Manual candle analysis failed.");
        }

        stockInfo.textContent =
            `${data.candles.length} manually entered candles analyzed`;

        renderPatterns(data.candles, data.patterns, patterns);
        results.style.display = "block";

    } catch (err) {
        error.textContent = err.message || "Unable to analyze manual candles.";
    } finally {
        loading.style.display = "none";
        button.disabled = false;
    }
}


document.addEventListener("DOMContentLoaded", () => {
    document.getElementById("symbol").addEventListener("keydown", event => {
        if (event.key === "Enter") analyzeStock();
    });

    document.getElementById("addCandleBtn").addEventListener("click", addCandleRow);

    document.querySelectorAll(".candle-input-row").forEach(row => {
        const removeButton = row.querySelector(".remove-candle-btn");

        if (removeButton) {
            removeButton.addEventListener("click", () => {
                row.remove();
                updateCandleNumbers();
            });
        }
    });
});
