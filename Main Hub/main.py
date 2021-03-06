from PySide import QtCore, QtGui
import sys
import os
import temp
from multiprocessing import Process, Queue
import datetime
import time

import mainhub
import pod

default_values = {}

started = False

#testing
count = 0
test_file = None

class Main(object):
	def __init__(self):
		self.ex = None
		self.app = None
		self.queue = Queue()
		self.p1 = None
		self.p2 = None
		self.set = False
		self.pod_list = []
		self.director = None
		self.map = ''
		self.available = False


	def gui_start(self):
		self.app = QtGui.QApplication(sys.argv)
		self.ex = temp.GUI()
		self.ex.confirm_signal.connect(self.push_queue)
		self.ex.close_signal.connect(self.close_system)
		self.ex.setWindowTitle("Spartan Superway Ticket System")  
		self.ex.setWindowFlags(self.ex.windowFlags() | QtCore.Qt.CustomizeWindowHint) 
		self.ex.setWindowFlags(self.ex.windowFlags() & ~QtCore.Qt.WindowMaximizeButtonHint)
		self.ex.setWindowFlags(self.ex.windowFlags() & ~QtCore.Qt.WindowMinimizeButtonHint)
		self.ex.show()
		sys.exit(self.app.exec_())


	def push_queue(self):
		self.queue.put((self.ex.source, self.ex.destination))
		print "Test"


	def close_system(self):
		self.queue.put(("system_close", None))
		self.ex.close_system()
		self.app.quit()

	def shutdown(self):
		self.director.close_system()
		for elem in self.pod_list:
			elem.close_system()	
		exit()


	def main_hub(self):
		self.director = mainhub.Director(2)

		for elem in range(self.director.get_pod()):
			self.pod_list.append(pod.Pod(elem))

		while(True):
			for elem in self.pod_list:
				if elem.get_run() == False:
					self.available = True
					break
				else:
					self.available = False
					print("All pods are in use")
					
			if (self.queue.empty()) or ((self.queue.empty() == False) and (self.available == False)):
				print("Current State: REPORT")
				for elem in self.pod_list:
					if elem.get_run() == True:
						package = self.director.transmit_package(str(bin(elem.get_id()+1)[2:].zfill(2)), '0100', '')
						try:
							self.director.check_package(package, elem.set_run)
							elem.set_source(elem.get_destination())
						except:
							pass
					else:
						elem.report()
			else:
				if (self.available == True):
					current = self.queue.get()
					if "system_close" in current:
						print("Current State: SHUTDOWN")
						self.shutdown()
					else:
						print("Current State: DIRECTOR")
						(source, destination) = current
						print("%s-%s" % (source, destination))
						self.director.set_source(source)
						self.director.set_destination(destination)
						directions = self.director.get_path()
						for elem in self.pod_list:
							if elem.get_run() == False and elem.get_location() == source:
								#directions = self.director.translate_path()
								self.director.reset_path()
								elem.set_run(True)
								#elem.logger(directions)
								self.parse_directions(directions)
								package = self.director.transmit_package(str(bin(elem.get_id()+1)[2:].zfill(2)), '0000', self.map.ljust(24,'0'))
								self.map = ''
								#Broken ASS code (A.rtifical S.upport S.ystem)
								try:
									self.director.check_package(package)
								except:
									print("Error in transmission")
									elem.set_run(False)
								break
							if elem.get_run() == False and elem.get_location() != source:
								self.director.set_source(elem.get_location())
								self.director.set_destination(source)
								self.queue.put(current)
								directions = self.director.translate_path()
								elem.set_run(True)
								elem.logger(directions)
								self.parse_directions(directions)
								package = self.director.transmit_package(str(bin(elem.get_id()+1)[2:].zfill(2)), '0000', self.map.ljust(24,'0'))
								self.map = ''
								#Broken ASS code (A.rtifical S.upport S.ystem)
								try:
									self.director.check_package(package)
								except:
									print("Invalid")
									elem.set_run(False)
								break
								

	def test(self):
		self.p1 = Process(target=self.main_hub)
		self.p2 = Process(target=self.gui_start)
		self.p1.start()
		time.sleep(3)
		self.p2.start()

	def parse_directions(self, directions):
		for elem in directions:
			self.map = self.map + elem


if __name__ == '__main__':
	maintest = Main()
	maintest.test()
