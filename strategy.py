#! /usr/local/bin/python2
import pandas as pd
import abc
import sqlite3
import numpy as np

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
		b = np.zeros(shape=(len(self.data["SPY"]),len(self.symbols)))
		self.signals = pd.DataFrame(b, columns=symbols)

	@abc.abstractmethod
	def build_strategy(self):
		pass

class Strat(AbstractStrategy):

	def build_strategy(self):
		pass

if __name__ == '__main__':

	s = Strat(["SPY", "TLT"], "/Users/peterpeluso/Desktop/bt/test.db", "2011-01-01", "2012-01-01")
	print s.data["SPY"]
