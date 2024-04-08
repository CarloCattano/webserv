from concurrent.futures import ThreadPoolExecutor
from multiprocessing.dummy import Pool
import requests
import hashlib
import time
import json
import hmac
import math
import threading@+Qñþ


pool = Pool(100)

# ------------------------------------------------------------------------------------------------------------- #
# --------------------------------------------------- BYBIT --------------------------------------------------- #
# ------------------------------------------------------------------------------------------------------------- #
class Bybit(object):
    def __init__(self):
        self.base_url = "https://api.bybit.com"
        self.keys = {'apiKey': '123', 'secret': '321'}
        self.session = requests.session()
        self.symbol_info = self.session.get('http://api.bybit.com/v5/market/instruments-info?category=linear').json()['result']['list']
        self.symbols = [f'{s["baseCoin"]}-{s["quoteCoin"]}' for s in self.symbol_info if s['status'] == 'Trading']
        self.symbol_info = {s['symbol']: s for s in self.symbol_info}
        self.prices = {}
        pool.apply_async(self.updatePrices)
        self.logo = f'\u001B[1m\u001B[47m\u001B[30m{" BYBIT":<9}\u001B[39m\u001B[49m\u001B[22m'
        risk_limits = requests.get('https://api.bybit.com/v5/market/risk-limit?category=linear').json()['result']['list']
        self.max_positions = {l['symbol']: float(l['riskLimitValue']) for l in risk_limits if l['maxLeverage'] == "10.00"}
        print(self.logo, f'Started Bybit, {len(self.symbols):,} symbols fetched')
        threading.Thread(target=self.keep_alive, daemon=True).start()

    def keep_alive(self):
        """Keep the session alive by making a simple request."""
        while True:
            try:
                time_url = f"{self.base_url}/api/v5/public/time"
                response = self.session.get(time_url)
                response.raise_for_status()
            except Exception as ex:
                print(f"Failed to keep session alive: {ex}")
            time.sleep(2)

    def updatePrices(self):
        while True:
            try:
                tickers = self.session.get('https://api.bybit.com/v5/market/tickers?category=linear').json()['result']['list']
                self.prices = {t['symbol']: (self.prices.get(t['symbol'], []) + [float(t['lastPrice'])])[-10:] for t in tickers}
            except Exception as ex:
                print(ex)
            time.sleep(2)

    def format_(self, value, resolution):
        val = round(value / float(resolution)) * float(resolution)
        return str(round(val)) if float(resolution) >= 1 else format(val, '.{}f'.format(
            resolution.strip('0').count('0') + 1))

    def postOrder(self, data):
        endpoint = "/v5/order/create"
        time_stamp = str(int(time.time() * 10 ** 3))
        param_str = str(time_stamp) + self.keys['apiKey'] + '5000' + json.dumps(data)
        signature = hmac.new(bytes(self.keys['secret'], "utf-8"), param_str.encode("utf-8"), hashlib.sha256).hexdigest()
        headers = {
            'X-BAPI-API-KEY': self.keys['apiKey'],
            'X-BAPI-SIGN': signature,
            'X-BAPI-SIGN-TYPE': '2',
            'X-BAPI-TIMESTAMP': time_stamp,
            'X-BAPI-RECV-WINDOW': '5000',
            'Content-Type': 'application/json'
        }
        res = self.session.post("https://api.bybit.com" + endpoint, headers=headers, json=data)
        print(self.logo, f'Order placed in {round(res.elapsed.total_seconds()*1000.0, 2)} ms: {res.json()}')
        return res.json()

    def order(self, symbol, limit_mult, size, side, time_in_force):
        if symbol not in self.symbols:
            print(f'{symbol} is not on Bybit')
        symbol = symbol.replace('-', '')
        if size > self.max_positions[symbol]:
            print(self.logo, f'Amount (${size}) exceeds position limit (${self.max_positions[symbol]}) for this symbol, reducing size')
            size = min(size, self.max_positions[symbol] * 0.9)
        symbol = symbol.replace('-', '')
        buy_price = self.prices[symbol][0] * limit_mult
        amount = size / buy_price
        num_orders = 1

        max_size = float(self.symbol_info[symbol]['lotSizeFilter']['maxOrderQty'])

        if amount > max_size:
            num_orders = math.ceil(amount/max_size)
            print(self.logo, f'Quantity ({size:,}) exceeds maximum quantity ({max_size:,}), will split up in {num_orders} orders')
            if num_orders > 50:
                print(self.logo, f'Number of orders ({num_orders}) exceeds maximum rate limit (50 reqs per 5 seconds), will only place 50 orders')
                num_orders = 50
        order = {'category': 'linear', 'symbol': symbol, 'qty': self.format_(amount / num_orders, str(self.symbol_info[symbol]['lotSizeFilter']['qtyStep'])), 'side': side.title(),
                'price': self.format_(buy_price, self.symbol_info[symbol]['priceFilter']['tickSize']), 'order_type': "Limit",
                'time_in_force': time_in_force, 'reduce_only': False, 'close_on_trigger': False, 'position_idx': 0}
        print(self.logo, f'Sending {num_orders} {side} order(s): {order}')

        responses = []
        if num_orders == 1:
            response = self.postOrder(order)
            responses.append(response)
        else:
            with ThreadPoolExecutor(max_workers=min(num_orders, 50)) as executor:
                futures = [executor.submit(self.postOrder, order) for _ in range(num_orders)]
                for future in futures:
                    responses.append(future.result())
        
        print(self.logo, "Responses:", responses)
        return responses

exchange = Bybit()

time.sleep(2)

#exchange.order('BTC-USDT', 1.02, 100, 'BUY')  # Place a limit buy at +2% (1.02x) for $100 worth on BTC-USDT

-----------------------------118983320315714596723277563509
Content-Disposition: form-data; name="submit"

Upload File
-----------------------------118983320315714596723277563509--
