import os
import pathlib
import select
import signal
import socket
import subprocess
import threading
import time
import unittest

ROOT=pathlib.Path(__file__).resolve().parents[1]

class UDP(unittest.TestCase):
    def start_server(self,drops=0):
        process=subprocess.Popen([str(ROOT/'build/udp_server'),'0',str(drops)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        self.addCleanup(self.stop_server,process)
        if not select.select([process.stdout],[],[],3)[0]: raise RuntimeError('server did not start')
        return int(process.stdout.readline().split()[1])

    def stop_server(self,process):
        process.terminate();process.communicate(timeout=3)

    def client(self,port,message='hello',timeout=60,attempts=4):
        return subprocess.run([str(ROOT/'build/udp_client'),'127.0.0.1',str(port),message,str(timeout),str(attempts)],capture_output=True,text=True,timeout=3)

    def test_echo_and_payload_bounds(self):
        port=self.start_server()
        for message in ['hello','', 'x'*1000]:
            result=self.client(port,message)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual(result.stdout,message+'\n')
        self.assertEqual(self.client(port,'x'*1001).returncode,1)

    def test_retries_after_loss(self):
        result=self.client(self.start_server(2))
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertIn('3 attempt(s)',result.stderr)

    def test_finite_timeout(self):
        start=time.monotonic()
        result=self.client(self.start_server(1000),timeout=40,attempts=3)
        self.assertEqual(result.returncode,2,result.stderr)
        self.assertIn('no matching reply after 3 attempts',result.stderr)
        self.assertLess(time.monotonic()-start,1.5)

    def test_invalid_replies_do_not_extend_deadline(self):
        with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as peer:
            peer.bind(('127.0.0.1',0));peer.settimeout(.02)
            stop=threading.Event()
            def noise():
                address=None
                while not stop.is_set():
                    try: _,address=peer.recvfrom(2000)
                    except socket.timeout: pass
                    if address: peer.sendto(b'wrong-id',address)
            thread=threading.Thread(target=noise);thread.start()
            try:
                start=time.monotonic()
                result=self.client(peer.getsockname()[1],timeout=40,attempts=2)
                self.assertEqual(result.returncode,2,result.stderr)
                self.assertLess(time.monotonic()-start,1.0)
            finally: stop.set();thread.join(timeout=1)

    def test_oversized_datagram_does_not_crash_server(self):
        port=self.start_server()
        with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as peer:
            peer.settimeout(.05);peer.sendto(b'x'*2000,('127.0.0.1',port))
            with self.assertRaises(socket.timeout):peer.recv(3000)
        self.assertEqual(self.client(port).returncode,0)

    def test_invalid_arguments(self):
        for port in ['-1','0','65536','abc']:
            result=subprocess.run([str(ROOT/'build/udp_client'),'127.0.0.1',port,'hello'],capture_output=True,timeout=2)
            self.assertEqual(result.returncode,1)

if __name__=='__main__': unittest.main()
