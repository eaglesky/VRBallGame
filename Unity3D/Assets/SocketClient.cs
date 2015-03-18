using UnityEngine;
using System;
using System.Collections;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

public class SocketClient : MonoBehaviour {
	
	// Use this for initialization
	
	//private const int listenPort = 5005;
	//public GameObject cameraObject;

	
	Thread receiveThread;
	UdpClient client;
	public int port;
	public string address = "127.0.0.1";
	//info
	
	public string lastReceivedUDPPacket = "";
	public string allReceivedUDPPackets = "";

	public Vector3 cameraPos = new Vector3(0, 0, -50);
	public Vector3 rotationAngles = new Vector3(0, 0, 0);
	public Vector3 mouthPos = new Vector3 (0, -10, -50);

	public float mouthRatio = 10;
	public Rect faceRectangle = new Rect(10, 10, 100, 100);

	void Start () {
		init();
	}
	
	void OnGUI(){
	/*	Rect  rectObj=new Rect (40,10,200,400);
		
		GUIStyle  style  = new GUIStyle ();
		
		style .alignment  = TextAnchor.UpperLeft;
		
		GUI .Box (rectObj,"# UDPReceive from " + address + ": " +port +"\n"
		          
		          + "Last Packet: \n"+ lastReceivedUDPPacket
		          
		          ,style );*/
		
	}
	
	private void init(){
		print ("UPDSend.init()");
		
		port = 5005;
		
		print ("Sending to " + address + ": " + port);
		
		receiveThread = new Thread (new ThreadStart(ReceiveData));
		receiveThread.IsBackground = true;
		receiveThread.Start ();
		
	}
	
	private void ReceiveData(){
		client = new UdpClient (port);
		while (true) {
			try{
				IPEndPoint anyIP = new IPEndPoint(IPAddress.Parse(address), port);
				byte[] data = client.Receive(ref anyIP);
				
				string text = Encoding.UTF8.GetString(data);
				//print (">> " + text);
				lastReceivedUDPPacket=text;
				allReceivedUDPPackets=allReceivedUDPPackets+text;
				string[] numberStrs = text.Split(' ');
				string debugNumbers = "";
				for (int i = 0; i < numberStrs.Length; ++i)
				{
					debugNumbers += numberStrs[i] + ", ";
				}

				print (debugNumbers);
				
				cameraPos.x = float.Parse(numberStrs[0]);
				cameraPos.y = float.Parse (numberStrs[1]);
				cameraPos.z = float.Parse (numberStrs[2]);

				rotationAngles.x = float.Parse (numberStrs[3]);
				rotationAngles.y = float.Parse (numberStrs[4]);
				rotationAngles.z = float.Parse (numberStrs[5]);

				mouthPos.x = float.Parse (numberStrs[6]);
				mouthPos.y = float.Parse (numberStrs[7]);
				mouthPos.z = float.Parse (numberStrs[8]);

				mouthRatio = float.Parse (numberStrs[9]);

				float faceRectangleX = float.Parse (numberStrs[10]) * Screen.width;
				float faceRectangleY = float.Parse(numberStrs[11]) * Screen.height;
				float faceRectangleWidth = float.Parse (numberStrs[12]) * Screen.width;
				float faceRectangleHeight = float.Parse(numberStrs[13]) * Screen.height;
				faceRectangle = new Rect(faceRectangleX, faceRectangleY, faceRectangleWidth, faceRectangleHeight);

				//print (faceRectangle);

			}catch(Exception e){
				print (e.ToString());
			}
		}
	}
	
	public string getLatestUDPPacket(){
		allReceivedUDPPackets = "";
		return lastReceivedUDPPacket;
	}
	
	// Update is called once per frame
	void Update () {
		//cameraObject.transform.position = cameraPos;
	}
	
	void OnApplicationQuit(){
		if (receiveThread != null) {
			receiveThread.Abort();
			Debug.Log(receiveThread.IsAlive); //must be false
		}
	}
}
