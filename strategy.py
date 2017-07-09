#! /usr/local/bin/python2
import pandas as pd
import abc
import sqlite3
import numpy as np
import statsmodels.formula.api as sm

class AbstractStrategy:

    __metaclass__ = abc.ABCMeta

    def __init__(self, symbols, path_to_db, start_date, end_date):
        self.symbols = symbols
        self.data = {}
        self.path_to_db = path_to_db 
        self.signals = None
        self.start_date = start_date
        self.end_date = end_date

        self.conn = sqlite3.connect(self.path_to_db)
        for symbol in symbols:
            self.data[symbol] = pd.read_sql_query("SELECT * FROM {0} WHERE Date > {1} and  Date < {2};".format(symbol, self.start_date, self.end_date), self.conn)
        b = np.zeros(shape=(len(self.data[self.symbols[0]]),len(self.symbols)))
        self.signals = pd.DataFrame(b, columns=symbols)
        self.signals["Date"] = self.data[symbols[0]]["Date"]
    @abc.abstractmethod
    def build_strategy(self):
        pass
    def to_db(self, table_name):
        self.signals.to_sql(table_name, self.conn, if_exists='replace', index=False)
    def close_db(self):
        self.conn.close()

class Strat(AbstractStrategy):

    def build_strategy(self):
        dat = pd.DataFrame()

        dat["RDS_A_pct"] = self.data["RDS_A"]["Close"].astype(float).pct_change()
        dat["RDS_B_pct"] = self.data["RDS_B"]["Close"].astype(float).pct_change()
        formula = "RDS_A_pct ~ RDS_B_pct"
        self.returns_reg_results = sm.ols(formula = formula, data= dat).fit()
        # self.returns_reg_results.params[1]
        self.data["spread"] = pd.DataFrame()
        self.data["spread"]["Close"] = self.data["RDS_A"]["Close"].astype(float) - (self.data["RDS_B"]["Close"].astype(float))
        mavg = self.data["spread"].rolling(50,min_periods=20).mean()
        mstd = self.data["spread"].rolling(100,min_periods=20).std()
        self.data["spread"]['mavg'] = mavg
        self.data["spread"]['mstd'] = mstd
        self.data["spread"]['buy_signals'] = 0
        self.data["spread"]['sell_signals'] = 0 
        self.data["spread"]["signal"] = 0 
        #self.data["spread"]['buy_signals'] = np.where((self.data["spread"] < (-2*self.data["spread"]["mstd"] + self.data["spread"]['mavg'])), 1,0)
        #print(np.where((self.data["spread"] > (2*self.data["spread"]["mstd"] + self.data["spread"]['mavg'])), -1,0))
        sig = 0
        leg = 0 
        print(self.returns_reg_results.params[1])
        for i in range(len(self.data['spread'])):
            if (self.data["spread"]["Close"][i] < (-2*self.data["spread"]["mstd"][i] + self.data["spread"]['mavg'][i])):
                if sig == 0:
                    self.data['spread']["signal"][i] = 1
                    print(1)
                    self.signals["RDS_A"][i] = 100
                    self.signals["RDS_B"][i] = -100 
                    sig = 1
                    leg = i
            if (self.data["spread"]["Close"][i] > (2*self.data["spread"]["mstd"][i] + self.data["spread"]['mavg'][i])):
                if sig == 0:
                    self.data['spread']["signal"][i] = -1
                    self.signals["RDS_A"][i] = -100
                    self.signals["RDS_B"][i] = 100                 
                    print(-1) 
                    sig = -1
                    leg = i 
            if abs(self.data["spread"]["Close"][i] - self.data["spread"]["mavg"][i]) < 0.1:
                if sig == 1:
                    self.data['spread']["signal"][i] = -1
                    self.signals["RDS_A"][i] = -100
                    self.signals["RDS_B"][i] = 100 
                    print(-2)
                    print("length of trade :", i - leg)
                    leg =0                    
                    sig = 0
                if sig == -1:
                    print(2)
                    self.data['spread']["signal"][i] = 1
                    self.signals["RDS_A"][i] = 100
                    self.signals["RDS_B"][i] = -100 
                    sig = 0
                    print("length of trade :", i - leg)
                    leg =0 


if __name__ == '__main__':

	s = Strat(["RDS_A", "RDS_B"], "test.db", "20070101", "20150101")
	s.build_strategy()
	print(s.signals)
	s.to_db("strat")
	s.close_db()

