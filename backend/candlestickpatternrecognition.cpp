#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <regex>
#include <sstream>
#include <iomanip>
#include <cstdlib>

using namespace std;

// ==========================================
// CANDLESTICKS - C++17
// ==========================================

struct Candle {
    double open, high, low, close, volume;
};

enum class Trend {
    UP, DOWN, SIDEWAYS, UNKNOWN
};

struct PatternResult {
    string name;
    string direction;
    string description;
    int startIndex;
    int endIndex;
};

vector<Candle> candles;
vector<PatternResult> results;

// ==========================================
// JSON INPUT
// ==========================================

bool getNumber(const string& object, const string& key, double& value) {
    regex pattern("\"" + key +
        "\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)");

    smatch match;

    if (!regex_search(object, match, pattern))
        return false;

    try {
        value = stod(match[1].str());
        return isfinite(value);
    }
    catch (...) {
        return false;
    }
}

string escapeJSON(const string& s) {
    string output;

    for (char c : s) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: output += c;
        }
    }

    return output;
}

bool validCandle(const Candle& c) {
    if (!isfinite(c.open) || !isfinite(c.high) ||
        !isfinite(c.low) || !isfinite(c.close) ||
        !isfinite(c.volume))
        return false;

    if (c.open <= 0 || c.high <= 0 ||
        c.low <= 0 || c.close <= 0 ||
        c.volume < 0)
        return false;

    return c.high >= max(c.open, c.close) &&
           c.low <= min(c.open, c.close) &&
           c.high >= c.low;
}

bool inputCandles() {
    string input(
        (istreambuf_iterator<char>(cin)),
        istreambuf_iterator<char>()
    );

    regex objectPattern("\\{[^}]*\\}");
    auto begin = sregex_iterator(
        input.begin(), input.end(), objectPattern
    );
    auto end = sregex_iterator();

    if (input.find('[') == string::npos ||
        input.find(']') == string::npos)
        return false;

    for (auto it = begin; it != end; ++it) {
        string object = it->str();

        Candle c{};

        if (!getNumber(object, "open", c.open) ||
            !getNumber(object, "high", c.high) ||
            !getNumber(object, "low", c.low) ||
            !getNumber(object, "close", c.close) ||
            !getNumber(object, "volume", c.volume))
            return false;

        if (!validCandle(c))
            return false;

        candles.push_back(c);
    }

    return !candles.empty();
}

// ==========================================
// BASIC CANDLE CALCULATIONS
// ==========================================

double body(const Candle& c) {
    return abs(c.close - c.open);
}

double candleRange(const Candle& c) {
    return c.high - c.low;
}

double upperWick(const Candle& c) {
    return c.high - max(c.open, c.close);
}

double lowerWick(const Candle& c) {
    return min(c.open, c.close) - c.low;
}

bool bullish(const Candle& c) {
    return c.close > c.open;
}

bool bearish(const Candle& c) {
    return c.close < c.open;
}

bool doji(const Candle& c) {
    return candleRange(c) > 0 &&
           body(c) <= candleRange(c) * 0.10;
}

bool longBody(const Candle& c) {
    return candleRange(c) > 0 &&
           body(c) >= candleRange(c) * 0.60;
}

bool shortBody(const Candle& c) {
    return candleRange(c) > 0 &&
           body(c) <= candleRange(c) * 0.30;
}

bool smallUpperWick(const Candle& c) {
    return upperWick(c) <= candleRange(c) * 0.10;
}

bool smallLowerWick(const Candle& c) {
    return lowerWick(c) <= candleRange(c) * 0.10;
}

bool longLowerWick(const Candle& c) {
    return lowerWick(c) >= body(c) * 2.0;
}

bool longUpperWick(const Candle& c) {
    return upperWick(c) >= body(c) * 2.0;
}

// ==========================================
// TREND IDENTIFICATION
// ==========================================

Trend identifyTrend(int index, int lookback = 5) {
    if (index < lookback)
        return Trend::UNKNOWN;

    double first = candles[index - lookback].close;
    double last = candles[index - 1].close;

    double change = ((last - first) / first) * 100.0;

    if (change > 1.0) return Trend::UP;
    if (change < -1.0) return Trend::DOWN;

    return Trend::SIDEWAYS;
}

// ==========================================
// PATTERN OUTPUT
// ==========================================

void addPattern(
    const string& name,
    const string& direction,
    const string& description,
    int start,
    int end
) {
    results.push_back({
        name, direction, description, start, end
    });
}

// ==========================================
// GAP DETECTION
// ==========================================

bool gapUp(int previous, int current) {
    return candles[current].low >
           candles[previous].high;
}

bool gapDown(int previous, int current) {
    return candles[current].high <
           candles[previous].low;
}

// ==========================================
// VOLUME ANALYSIS
// ==========================================

bool volumeAboveAverage(int index, int period = 5) {
    if (index < period)
        return false;

    double total = 0;

    for (int i = index - period; i < index; i++)
        total += candles[i].volume;

    return candles[index].volume > total / period;
}

// ==========================================
// PATTERN HELPERS
// ==========================================

bool engulfingBullish(int i) {
    if (i < 1) return false;

    Candle a = candles[i - 1];
    Candle b = candles[i];

    return bearish(a) && bullish(b) &&
           b.open <= a.close &&
           b.close >= a.open &&
           body(b) > body(a);
}

bool engulfingBearish(int i) {
    if (i < 1) return false;

    Candle a = candles[i - 1];
    Candle b = candles[i];

    return bullish(a) && bearish(b) &&
           b.open >= a.close &&
           b.close <= a.open &&
           body(b) > body(a);
}

bool haramiBullish(int i) {
    if (i < 1) return false;

    Candle a = candles[i - 1];
    Candle b = candles[i];

    return bearish(a) && bullish(b) &&
           b.open > a.close &&
           b.close < a.open;
}

bool haramiBearish(int i) {
    if (i < 1) return false;

    Candle a = candles[i - 1];
    Candle b = candles[i];

    return bullish(a) && bearish(b) &&
           b.open > a.open &&
           b.close < a.close;
}

bool hammerShape(const Candle& c) {
    return longLowerWick(c) &&
           upperWick(c) <= body(c) * 0.5;
}

bool shootingStarShape(const Candle& c) {
    return longUpperWick(c) &&
           lowerWick(c) <= body(c) * 0.5;
}

// ==========================================
// SINGLE-CANDLE PATTERNS
// ==========================================

void detectSinglePatterns() {
    for (int i = 0; i < (int)candles.size(); i++) {
        Candle c = candles[i];
        Trend t = identifyTrend(i);

        if (doji(c))
            addPattern("Doji", "Neutral",
                "Open and close prices are nearly equal.", i, i);

        if (doji(c) && lowerWick(c) <= candleRange(c) * 0.10)
            addPattern("Dragonfly Doji", "Potentially Bullish",
                "Doji with a long lower shadow.", i, i);

        if (doji(c) && upperWick(c) <= candleRange(c) * 0.10)
            addPattern("Gravestone Doji", "Potentially Bearish",
                "Doji with a long upper shadow.", i, i);

        if (doji(c) && upperWick(c) > body(c) &&
            lowerWick(c) > body(c))
            addPattern("Long-Legged Doji", "Neutral",
                "Doji with long shadows on both sides.", i, i);

        if (hammerShape(c) && t == Trend::DOWN)
            addPattern("Hammer", "Bullish",
                "Long lower shadow following a downtrend.", i, i);

        if (hammerShape(c) && t == Trend::UP)
            addPattern("Hanging Man", "Bearish",
                "Hammer-shaped candle following an uptrend.", i, i);

        if (shootingStarShape(c) && t == Trend::UP)
            addPattern("Shooting Star", "Bearish",
                "Long upper shadow following an uptrend.", i, i);

        if (shootingStarShape(c) && t == Trend::DOWN)
            addPattern("Inverted Hammer", "Potentially Bullish",
                "Inverted hammer shape following a downtrend.", i, i);

        if (candleRange(c) > 0 &&
            body(c) <= candleRange(c) * 0.10 &&
            upperWick(c) > candleRange(c) * 0.30 &&
            lowerWick(c) > candleRange(c) * 0.30)
            addPattern("High Wave", "Neutral",
                "Small body with unusually long shadows.", i, i);

        if (longBody(c) && bullish(c))
            addPattern("Long White Candle", "Bullish",
                "Long bullish real body.", i, i);

        if (longBody(c) && bearish(c))
            addPattern("Long Black Candle", "Bearish",
                "Long bearish real body.", i, i);

        if (shortBody(c) && bullish(c))
            addPattern("Short White Candle", "Bullish",
                "Short bullish real body.", i, i);

        if (shortBody(c) && bearish(c))
            addPattern("Short Black Candle", "Bearish",
                "Short bearish real body.", i, i);

        if (bullish(c) && smallUpperWick(c) &&
            smallLowerWick(c))
            addPattern("White Marubozu", "Bullish",
                "Bullish candle with almost no shadows.", i, i);

        if (bearish(c) && smallUpperWick(c) &&
            smallLowerWick(c))
            addPattern("Black Marubozu", "Bearish",
                "Bearish candle with almost no shadows.", i, i);

        if (candleRange(c) > 0 &&
            body(c) <= candleRange(c) * 0.30 &&
            upperWick(c) > body(c) &&
            lowerWick(c) > body(c))
            addPattern("Spinning Top", "Neutral",
                "Small body with shadows on both sides.", i, i);

        if (volumeAboveAverage(i))
            addPattern("Above-Average Volume", "Volume Signal",
                "Volume exceeds the preceding five-candle average.",
                i, i);
    }
}

// ==========================================
// TWO-CANDLE PATTERNS
// ==========================================

void detectTwoCandlePatterns() {
    for (int i = 1; i < (int)candles.size(); i++) {
        Candle a = candles[i - 1];
        Candle b = candles[i];

        Trend t = identifyTrend(i - 1);

        if (engulfingBullish(i))
            addPattern("Bullish Engulfing", "Bullish",
                "Second bullish body engulfs the first bearish body.",
                i - 1, i);

        if (engulfingBearish(i))
            addPattern("Bearish Engulfing", "Bearish",
                "Second bearish body engulfs the first bullish body.",
                i - 1, i);

        if (haramiBullish(i) && t == Trend::DOWN)
            addPattern("Bullish Harami", "Bullish",
                "Small bullish body contained within previous bearish body.",
                i - 1, i);

        if (haramiBearish(i) && t == Trend::UP)
            addPattern("Bearish Harami", "Bearish",
                "Small bearish body contained within previous bullish body.",
                i - 1, i);

        if (haramiBullish(i) && doji(b))
            addPattern("Bullish Harami Cross", "Potentially Bullish",
                "Doji contained within a previous bearish body.",
                i - 1, i);

        if (haramiBearish(i) && doji(b))
            addPattern("Bearish Harami Cross", "Potentially Bearish",
                "Doji contained within a previous bullish body.",
                i - 1, i);

        if (bearish(a) && bullish(b) &&
            b.open < a.low &&
            b.close > (a.open + a.close) / 2.0 &&
            b.close < a.open)
            addPattern("Piercing Pattern", "Bullish",
                "Bullish candle closes above the midpoint of the prior body.",
                i - 1, i);

        if (bullish(a) && bearish(b) &&
            b.open > a.high &&
            b.close < (a.open + a.close) / 2.0 &&
            b.close > a.open)
            addPattern("Dark Cloud Cover", "Bearish",
                "Bearish candle closes below the midpoint of the prior body.",
                i - 1, i);

        if (abs(a.low - b.low) <=
            max(candleRange(a), candleRange(b)) * 0.03)
            addPattern("Tweezers Bottom Candidate", "Potentially Bullish",
                "Two candles have approximately matching lows.",
                i - 1, i);

        if (abs(a.high - b.high) <=
            max(candleRange(a), candleRange(b)) * 0.03)
            addPattern("Tweezers Top Candidate", "Potentially Bearish",
                "Two candles have approximately matching highs.",
                i - 1, i);

        if (gapUp(i - 1, i))
            addPattern("Rising Window", "Bullish",
                "Current candle's low is above the previous high.",
                i - 1, i);

        if (gapDown(i - 1, i))
            addPattern("Falling Window", "Bearish",
                "Current candle's high is below the previous low.",
                i - 1, i);

        if (bullish(a) && bearish(b) &&
            abs(a.high - b.high) <=
            max(candleRange(a), candleRange(b)) * 0.03)
            addPattern("Bearish Tweezer Top", "Bearish",
                "Bullish and bearish candles with matching highs.",
                i - 1, i);

        if (bearish(a) && bullish(b) &&
            abs(a.low - b.low) <=
            max(candleRange(a), candleRange(b)) * 0.03)
            addPattern("Bullish Tweezer Bottom", "Bullish",
                "Bearish and bullish candles with matching lows.",
                i - 1, i);
    }
}

// ==========================================
// THREE-CANDLE PATTERNS
// ==========================================

void detectThreeCandlePatterns() {
    for (int i = 2; i < (int)candles.size(); i++) {
        Candle a = candles[i - 2];
        Candle b = candles[i - 1];
        Candle c = candles[i];

        Trend t = identifyTrend(i - 2);

        double midpointA = (a.open + a.close) / 2.0;

        if (bearish(a) && longBody(a) &&
            shortBody(b) && bullish(c) &&
            longBody(c) && c.close > midpointA)
            addPattern("Morning Star", "Bullish",
                "Bearish candle, small middle body, and strong bullish reversal.",
                i - 2, i);

        if (bullish(a) && longBody(a) &&
            shortBody(b) && bearish(c) &&
            longBody(c) && c.close < midpointA)
            addPattern("Evening Star", "Bearish",
                "Bullish candle, small middle body, and strong bearish reversal.",
                i - 2, i);

        if (bearish(a) && longBody(a) &&
            doji(b) && bullish(c) &&
            longBody(c) && c.close > midpointA)
            addPattern("Morning Doji Star", "Bullish",
                "Morning Star variation with a middle Doji.",
                i - 2, i);

        if (bullish(a) && longBody(a) &&
            doji(b) && bearish(c) &&
            longBody(c) && c.close < midpointA)
            addPattern("Evening Doji Star", "Bearish",
                "Evening Star variation with a middle Doji.",
                i - 2, i);

        if (bullish(a) && bullish(b) && bullish(c) &&
            a.close < b.close && b.close < c.close &&
            a.open < b.open && b.open < c.open)
            addPattern("Three White Soldiers", "Bullish",
                "Three consecutive rising bullish candles.",
                i - 2, i);

        if (bearish(a) && bearish(b) && bearish(c) &&
            a.close > b.close && b.close > c.close &&
            a.open > b.open && b.open > c.open)
            addPattern("Three Black Crows", "Bearish",
                "Three consecutive falling bearish candles.",
                i - 2, i);

        if (doji(a) && doji(b) && doji(c))
            addPattern("Tri-Star Candidate", "Potential Reversal",
                "Three consecutive Doji candles; gap structure needs verification.",
                i - 2, i);

        if (bearish(a) && bullish(b) && bearish(c) &&
            abs(a.close - c.close) <=
            max(candleRange(a), candleRange(c)) * 0.03)
            addPattern("Stick Sandwich Candidate", "Potentially Bullish",
                "Bullish middle candle between bearish candles with similar closes.",
                i - 2, i);

        if (bullish(a) && bearish(b) && bullish(c) &&
            c.close > a.close && c.open > b.close)
            addPattern("Three Inside Up Candidate", "Potentially Bullish",
                "Harami-like formation followed by bullish confirmation.",
                i - 2, i);

        if (bearish(a) && bullish(b) && bearish(c) &&
            c.close < a.close && c.open < b.close)
            addPattern("Three Inside Down Candidate", "Potentially Bearish",
                "Harami-like formation followed by bearish confirmation.",
                i - 2, i);

        if (bearish(a) && bullish(b) && bullish(c) &&
            c.close > b.close && b.close > a.open)
            addPattern("Three Outside Up Candidate", "Potentially Bullish",
                "Bullish engulfing-like formation followed by confirmation.",
                i - 2, i);

        if (bullish(a) && bearish(b) && bearish(c) &&
            c.close < b.close && b.close < a.open)
            addPattern("Three Outside Down Candidate", "Potentially Bearish",
                "Bearish engulfing-like formation followed by confirmation.",
                i - 2, i);

        if (t == Trend::DOWN &&
            bearish(a) && shortBody(b) && bullish(c) &&
            c.close > a.open)
            addPattern("Abandoned Baby Candidate", "Potentially Bullish",
                "Potential reversal formation; isolated middle candle gap must be verified.",
                i - 2, i);

        if (t == Trend::UP &&
            bullish(a) && shortBody(b) && bearish(c) &&
            c.close < a.open)
            addPattern("Abandoned Baby Candidate", "Potentially Bearish",
                "Potential reversal formation; isolated middle candle gap must be verified.",
                i - 2, i);
    }
}

// ==========================================
// DETECTION PIPELINE
// ==========================================

void runDetection() {
    results.clear();

    detectSinglePatterns();
    detectTwoCandlePatterns();
    detectThreeCandlePatterns();
}

// ==========================================
// JSON OUTPUT
// ==========================================

void displayResults() {
    cout << "{\"success\":true,\"patterns\":[";

    for (size_t i = 0; i < results.size(); i++) {
        const auto& p = results[i];

        if (i > 0)
            cout << ",";

        cout << "{"
             << "\"name\":\"" << escapeJSON(p.name) << "\","
             << "\"direction\":\"" << escapeJSON(p.direction) << "\","
             << "\"description\":\"" << escapeJSON(p.description) << "\","
             << "\"startIndex\":" << p.startIndex << ","
             << "\"endIndex\":" << p.endIndex
             << "}";
    }

    cout << "],\"totalDetections\":" << results.size() << "}";
}

// ==========================================
// MAIN
// ==========================================

int main() {
    if (!inputCandles()) {
        cout << "{\"success\":false,\"error\":\"Invalid or missing candle JSON data\"}";
        return 1;
    }

    runDetection();
    displayResults();

    return 0;
}