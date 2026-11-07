
from flask import Flask, request, jsonify, send_from_directory
from pathlib import Path
from server import get_stock_data
import subprocess
import json

app = Flask(__name__)

BACKEND_DIR = Path(__file__).resolve().parent
PROJECT_DIR = BACKEND_DIR.parent


def find_frontend():
    frontend = PROJECT_DIR / "frontend"

    if (frontend / "index.html").is_file():
        return frontend

    return None


FRONTEND_DIR = find_frontend()


def find_executable():
    possible_names = [
        "Candlestick.exe",
        "candlestick.exe",
        "candlesticks.exe"
    ]

    for name in possible_names:
        path = BACKEND_DIR / name
        if path.is_file():
            return path

    return None


def run_cpp_analysis(candles):
    executable = find_executable()

    if executable is None:
        raise RuntimeError(
            "Candlestick.exe was not found inside the backend folder."
        )

    try:
        result = subprocess.run(
            [str(executable)],
            input=json.dumps(candles),
            capture_output=True,
            text=True,
            timeout=30,
            cwd=str(BACKEND_DIR)
        )
    except Exception as error:
        raise RuntimeError(f"Could not start C++ program: {error}")

    if result.returncode != 0:
        raise RuntimeError(
            f"C++ program exited with code {result.returncode}. "
            f"Error output: {result.stderr or 'No error output'}. "
            f"Program output: {result.stdout or 'No program output'}"
        )

    try:
        output = json.loads(result.stdout)
    except json.JSONDecodeError:
        raise RuntimeError(
            "C++ program did not return valid JSON. "
            f"Output received: {result.stdout[:1500]}"
        )

    if not output.get("success"):
        raise RuntimeError(
            output.get("error", "C++ pattern detection failed.")
        )

    return output


@app.route("/")
def home():
    if FRONTEND_DIR is None:
        return "Frontend folder not found.", 404

    return send_from_directory(str(FRONTEND_DIR), "index.html")


@app.route("/<path:filename>")
def frontend_files(filename):
    if FRONTEND_DIR is None:
        return "Frontend folder not found.", 404

    file_path = FRONTEND_DIR / filename

    if not file_path.is_file():
        return "File not found.", 404

    return send_from_directory(str(FRONTEND_DIR), filename)


@app.route("/analyse", methods=["POST"])
def analyse():
    try:
        data = request.get_json(silent=True) or {}
        symbol = str(data.get("symbol", "")).strip().upper()

        if not symbol:
            return jsonify({"error": "Enter a stock symbol."}), 400

        candles = get_stock_data(symbol)

        if not candles:
            return jsonify({"error": "No stock data found."}), 404

        output = run_cpp_analysis(candles)

        return jsonify({
            "success": True,
            "symbol": symbol,
            "candles": candles,
            "patterns": output
        })

    except Exception as error:
        print("STOCK ANALYSIS ERROR:", str(error))
        return jsonify({"error": str(error)}), 500


@app.route("/analyse-candles", methods=["POST"])
def analyse_manual_candles():
    try:
        data = request.get_json(silent=True) or {}
        candles = data.get("candles")

        if not isinstance(candles, list) or not candles:
            return jsonify({"error": "Enter at least one candle."}), 400

        output = run_cpp_analysis(candles)

        return jsonify({
            "success": True,
            "candles": candles,
            "patterns": output
        })

    except Exception as error:
        print("MANUAL ANALYSIS ERROR:", str(error))
        return jsonify({"error": str(error)}), 500


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=False)
