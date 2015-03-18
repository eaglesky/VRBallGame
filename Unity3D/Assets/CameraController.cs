using UnityEngine;
using System;
using System.Collections;
using System.Text;
using System.IO;  

public class CameraController : MonoBehaviour {
	public SocketClient socketClient;
	public GameObject boxBottom;
	public GameObject boxTop;
	public GameObject boxLeft;
	public GameObject boxRight;
	public GameObject boxBack;

	public GameObject frameTop;
	public GameObject frameBottom;
	public GameObject frameRight;
	public GameObject frameLeft;

	public GameObject ball;
	public GameObject mouthBeam;

	public GameObject TargetPlane;
	public GameObject TargetPole;
	public GameObject breakGlassSprite;
	public GameObject missile;

	public float left = -0.2F;
	public float right = 0.2F;
	public float top = 0.2F;
	public float bottom = -0.2F;

	public float screenWidth = 34.5F;
	public float screenHeight = 19.5F;

	public Texture faceImage;
	public Texture startScene;

	public Vector2 cursorPos;

	static public bool ballInPlay;// { get { return ballInPlay; } set {} }
	public bool ballFallOut = false;
	public bool clearStage = false;
	public bool startGame = false;
	
	static public float boxLength;// { get { return boxLength; } set {} }
	
	private float missileTimer;

	private float thickNess;

	public GUISkin textSkin;

	//private LineRenderer line;
	public Color lineColor = Color.blue;
	//public float lineSize = .2f;

	private float mouthHitThreshold = 4.5f;

	// Use this for initialization
	void Start () {
		GetComponent<Camera>().nearClipPlane = 0.5f;
		GetComponent<Camera>().farClipPlane = 1000;
		print (Application.dataPath);
		string filePath = Application.dataPath + "\\..\\..\\OpenCV\\my_parameters.txt";

		LoadFile (filePath);


		setBoxSize ();
		//print (boxBottom.GetComponent<Renderer> ().bounds);
		//print (boxLeft.GetComponent<Renderer> ().bounds);
		
		/*line = gameObject.AddComponent<LineRenderer>();
		line.SetColors(lineColor, lineColor);
		line.SetVertexCount(2);
		line.SetWidth(lineSize, lineSize);
		line.material = mat;*/

		mouthBeam.transform.position = new Vector3 (0, 0, 0);
		mouthBeam.transform.localScale = new Vector3 (1f, 1f, 1f);
		//mouthBeam.GetComponent<Renderer> ().material = mat;

		missileTimer = UnityEngine.Random.Range (1F, 5F);
		ballInPlay = true;
		ball.SetActive (false);
		mouthBeam.SetActive (false);
	}

	void setTarget() {
		GameObject[] target = GameObject.FindGameObjectsWithTag("Target");
		GameObject[] targetPole = GameObject.FindGameObjectsWithTag("TargetPole");
		for (int i = 0; i < target.GetLength(0); ++i)
			Destroy (target [i]);
		for (int i = 0; i < targetPole.GetLength(0); ++i)
			Destroy (targetPole [i]);
		
		for (int i = 0; i < 7; ++i) {
			Vector3 position = new Vector3 (UnityEngine.Random.Range (-screenWidth/2-thickNess/2+2.5F, screenWidth/2-thickNess/2-2.5F), 
			                                UnityEngine.Random.Range (2.5F, screenHeight+thickNess/2-2.5f), 
			                                UnityEngine.Random.Range (-boxLength/2, boxLength/2));
			Vector3 polePos = new Vector3 (position.x, position.y - 22F, position.z);
			
			Instantiate (TargetPlane, position, Quaternion.Euler (new Vector3 (0.0F, 0.0F, 0.0F)));
			Instantiate (TargetPole, polePos, Quaternion.Euler (new Vector3 (0.0F, 0.0F, 0.0F)));
		}
	}

	void setBall() {
		ball.transform.position = new Vector3 (0, screenHeight/2F, -boxLength/2F);
		ball.transform.forward=new Vector3(0,0,1);  
		ball.transform.GetComponent<Rigidbody>().velocity = new Vector3();
		ball.transform.GetComponent<Rigidbody>().AddForce (transform.forward * 20F, ForceMode.Impulse);
	}

	void setBoxSize() {
		thickNess = 0.6f;
		boxLength = 100;

		boxBottom.transform.position = new Vector3 (0, 0, 0);
		boxBottom.transform.eulerAngles = new Vector3 (0, 0, 0);
		boxBottom.transform.localScale = new Vector3 (1, 1, 1);
		Vector3 curBounds = boxBottom.GetComponent<Renderer> ().bounds.extents;

		boxBottom.transform.localScale = new Vector3 ((screenWidth / 2 + thickNess)/curBounds.x, 
		                                              (thickNess / 2)/curBounds.y, 
		                                              (boxLength / 2 + thickNess)/curBounds.z);

		boxTop.transform.position = new Vector3 (0, screenHeight + thickNess, 0);
		boxTop.transform.eulerAngles = boxBottom.transform.eulerAngles;
		boxTop.transform.localScale = boxBottom.transform.localScale;
		frameTop.transform.position = boxTop.transform.position;
		frameTop.transform.localScale = new Vector3 ((screenWidth / 2 + thickNess)/curBounds.x,
		                                             (thickNess)/curBounds.y,
		                                             (thickNess)/curBounds.y);
		//frameBottom.transform.position = new Vector3 ();
		frameBottom.transform.localScale = new Vector3 ((screenWidth / 2 + thickNess)/curBounds.x,
		                                             (thickNess)/curBounds.y,
		                                             (thickNess)/curBounds.y);

		boxLeft.transform.position = new Vector3 (-screenWidth / 2 - thickNess / 2, 
		                                         screenHeight / 2 + thickNess / 2,
		                                         0);
		boxLeft.transform.eulerAngles = new Vector3 (0, 0, 90);
		boxLeft.transform.localScale = new Vector3(1, 1, 1);
		curBounds = boxLeft.GetComponent<Renderer> ().bounds.extents;
		boxLeft.transform.localScale = new Vector3 ((screenHeight / 2 + thickNess)/curBounds.x, 
		                                           (thickNess / 2)/curBounds.y,
		                                           (boxLength / 2 + thickNess)/curBounds.z);
		frameLeft.transform.position = boxLeft.transform.position;
		frameLeft.transform.localScale = new Vector3 ((thickNess)/curBounds.y,
		                                              (screenHeight / 2 + thickNess)/curBounds.x,
		                                              (thickNess)/curBounds.y);

		boxRight.transform.position = new Vector3 (-boxLeft.transform.position.x, 
		                                          boxLeft.transform.position.y,
		                                          boxLeft.transform.position.z);
		boxRight.transform.eulerAngles = boxLeft.transform.eulerAngles;
		boxRight.transform.localScale = boxLeft.transform.localScale;
		frameRight.transform.position = boxRight.transform.position;
		frameRight.transform.localScale = new Vector3 ((thickNess)/curBounds.y,
		                                              (screenHeight / 2 + thickNess)/curBounds.x,
		                                              (thickNess)/curBounds.y);

		boxBack.transform.position = new Vector3 (0, screenHeight / 2 + thickNess / 2,
		                                          boxLength/2 + thickNess / 2);
		boxBack.transform.eulerAngles = new Vector3 (90, 0, 0);
		boxBack.transform.localScale = new Vector3 (1, 1, 1);
		curBounds = boxBack.GetComponent<Renderer> ().bounds.extents;
		boxBack.transform.localScale = new Vector3 ((screenWidth / 2 + thickNess)/curBounds.x,
		                                           (thickNess / 2)/curBounds.y,
		                                           (screenHeight / 2 + thickNess)/curBounds.z);

	}

	void restartGame(bool resetThings)
	{
		GameObject[] sprite = GameObject.FindGameObjectsWithTag ("GlassSprite");
		GameObject[] missiles = GameObject.FindGameObjectsWithTag ("Missile");
		if (resetThings) {
			for (int i = 0; i < sprite.GetLength(0); ++i)
				Destroy (sprite [i]);
		}
		for (int i = 0; i < missiles.GetLength(0); ++i) {
			Destroy (missiles [i]);
		}

		missileTimer = UnityEngine.Random.Range (3F, 10F);
		ballInPlay = true;
		ball.SetActive (true);
		if (resetThings)
			ball.GetComponent<BallManager> ().score = 0;
		setBall ();
	}
		
	void Update()
	{
		if (startGame) {
			GameObject[] target = GameObject.FindGameObjectsWithTag ("Target");
			GameObject[] targetPole = GameObject.FindGameObjectsWithTag ("TargetPole");
			if (target.GetLength (0) == 0) {
				clearStage = true;
				return;
			}


			if (ball.transform.position.z < -boxLength / 2F && ballInPlay) {
				Instantiate (breakGlassSprite, ball.transform.position, Quaternion.Euler (new Vector3 (0.0F, 0.0F, 0.0F)));
				ballInPlay = false;
				ballFallOut = true;
			}
			else if (ball.activeSelf && ball.transform.position.z >= -boxLength / 2F 
			         && ball.transform.position.x > -screenWidth/2F && ball.transform.position.x < screenWidth/2F
			         && ball.transform.position.y > 0               && ball.transform.position.y < screenHeight) {
				ballFallOut = false;
				ballInPlay = true;
			}

			if (Input.GetKey (KeyCode.R)) {
				restartGame (false);
				ballFallOut = false;
			}

			missileTimer -= Time.deltaTime;
			if (missileTimer < 0 && ballInPlay) {
				if (transform.position.x < screenWidth / 2 - 1 && transform.position.x > -screenWidth / 2 + 1 &&
				    transform.position.y > 1 && transform.position.y < screenHeight - 1) {
					Instantiate (missile, new Vector3 (transform.position.x, transform.position.y, boxLength / 2F),
				            Quaternion.Euler (new Vector3 (0.0F, 90.0F, 0.0F)));
					missileTimer = UnityEngine.Random.Range (7F, 15F);
				}
			}
		}
	}
	

	void OnGUI() {
		if (!faceImage) {
			Debug.LogError("Assign a Texture in the inspector.");
			return;
		}

		//print (cursorPos);
		//GUI.Box(new Rect(10, 0, 100, 100), faceImage);
		GUI.skin = textSkin;



		if (!startGame) {
			GUI.DrawTexture(new Rect(Screen.width / 2 - 442, 100, 885, 300), startScene, ScaleMode.StretchToFill);
			if (GUI.Button (new Rect (Screen.width / 2 - 120, Screen.height / 2 + 150, 240, 75), "Start Game!")) {
				startGame = true;
				ball.SetActive (true);
				mouthBeam.SetActive(true);
				setBall ();
				setTarget ();
			}
		} else {
			GUI.DrawTexture(socketClient.faceRectangle, faceImage, ScaleMode.StretchToFill);
			if (clearStage) {
				String gameover = "Congratulations!\nYou cleared all targets!";
				GUI.skin.label.fontSize = 80;
				GUI.Label (new Rect (0, Screen.height / 2 - 100, Screen.width, 200), gameover);
				if (GUI.Button (new Rect (Screen.width / 2 - 100, Screen.height / 2 + 150, 200, 75), "Next round!")) {
					setTarget ();
					restartGame (true);
					clearStage = false;
					ballFallOut = false;
				}
			} else if (ballInPlay) {
				GUI.skin.label.fontSize = 40;
				String scoreStr = "Score : " + ball.GetComponent<BallManager> ().score.ToString ();
				GUI.Label (new Rect (0, 0, Screen.width, 100), scoreStr);
			} else {
				if (!ballFallOut) {
					String gameover = "Game over!\nYour score: " + ball.GetComponent<BallManager> ().score.ToString ();
					GUI.skin.label.fontSize = 80;
					GUI.Label (new Rect (0, Screen.height / 2 - 100, Screen.width, 200), gameover);
					GUI.skin.button.fontSize = 40;
					if (GUI.Button (new Rect (Screen.width / 2 - 100, Screen.height / 2 + 150, 200, 75), "Restart")) {
						setTarget();
						restartGame (true);
					}
				}
				else {
					String gameover = "Ball is out!";
					GUI.skin.label.fontSize = 80;
					GUI.Label (new Rect (0, Screen.height / 2 - 100, Screen.width, 200), gameover);
					GUI.skin.button.fontSize = 40;
					if (GUI.Button (new Rect (Screen.width / 2 - 150, Screen.height / 2 + 150, 300, 75), "Reset the ball")) {
						restartGame (false);
						ballFallOut = false;
					}
				}
			}
		}
	}

	// Update is called once per frame
	void LateUpdate () {
		Vector3 cameraPosSocket = socketClient.cameraPos;
		transform.position = new Vector3 (cameraPosSocket.x, 
		                                 cameraPosSocket.y + screenHeight / 2 + thickNess / 2,
		                                 boxBack.transform.position.z + cameraPosSocket.z - boxLength - 1);

		Camera cam = GetComponent<Camera> ();

		float ratio = cam.nearClipPlane / (-cameraPosSocket.z);
		left = (-screenWidth / 2.0f - cameraPosSocket.x) * ratio;
		right = (screenWidth / 2.0f - cameraPosSocket.x) * ratio;
		top = (screenHeight / 2.0f - cameraPosSocket.y) * ratio;
		bottom = (-screenHeight / 2.0f - cameraPosSocket.y) * ratio;

		Matrix4x4 m = PerspectiveOffCenter(left, right, bottom, top, cam.nearClipPlane, cam.farClipPlane);
		cam.projectionMatrix = m;

		Vector3 angleRadian = socketClient.rotationAngles;
		angleRadian.x = -angleRadian.x;
		angleRadian.y = -angleRadian.y;

		/*Vector2 cursorClipPlane = new Vector2 (0, 0);
		cursorClipPlane.y = top - cam.nearClipPlane * Mathf.Tan (angleRadian.x);
		cursorClipPlane.x = cam.nearClipPlane * Mathf.Tan (angleRadian.y) - left;

		cursorPos.x = cursorClipPlane.x * (Screen.width / (right - left));
		cursorPos.y = cursorClipPlane.y * (Screen.height / (top - bottom));*/
		//print (Screen.width + " x " + Screen.height);

		angleRadian.x -= 0.04f;
		Vector3 mouthPosSocket = socketClient.mouthPos;
		Vector3 mouthEyeDisCamera = (mouthPosSocket - cameraPosSocket) * ratio;
		Vector3 mouthPos = mouthEyeDisCamera + transform.position;
		Vector3 mouthProjected = mouthPos;
		mouthProjected.z += cam.nearClipPlane;
		mouthProjected.y += cam.nearClipPlane * Mathf.Tan (angleRadian.x);
		mouthProjected.x += cam.nearClipPlane * Mathf.Tan (angleRadian.y);


		/*Vector3 mouthPos = mouthPosSocket - cameraPosSocket + transform.position;
		Vector3 mouthProjected = mouthPos;

		mouthProjected.z = mouthPos.z - mouthPosSocket.z;
		mouthProjected.y += -mouthPosSocket.z* Mathf.Tan (angleRadian.x);
		mouthProjected.x += -mouthPosSocket.z * Mathf.Tan (angleRadian.y);*/

		float mouthLineLength = boxLength - mouthPos.z;
		Vector3 mouthDis = (mouthProjected - mouthPos) * mouthLineLength;
		Vector3 endPoint = mouthDis + mouthProjected;



		mouthBeam.transform.position = mouthProjected + mouthDis / 2.0f;
		Vector3 mouthBeamScale = mouthBeam.transform.localScale;
		//Vector3 curBounds = mouthBeam.GetComponent<Renderer> ().bounds.extents;
		//mouthBeamScale.y = mouthDis.magnitude / curBounds.y;
		mouthBeamScale.y = mouthDis.magnitude * 0.4f;

		mouthBeam.transform.localScale = mouthBeamScale;

		mouthBeam.transform.LookAt (endPoint);
		mouthBeam.transform.RotateAround (mouthBeam.transform.position, mouthBeam.transform.right, 90);

		//Check hit!
		if (socketClient.mouthRatio <= mouthHitThreshold) {
			mouthBeam.GetComponent<Renderer> ().material.color = Color.yellow;
			Vector3 mouthForceNormalized = endPoint - mouthBeam.transform.position;
			mouthForceNormalized.Normalize();
			Vector3 ballDir = ball.transform.position - mouthPos;
			float ballAngle = Vector3.Angle(ballDir, mouthForceNormalized);
			//print (ballAngle);

			if (ballAngle < 10) {
				float ballMouthDis = ballDir.magnitude;
				print (ballMouthDis);
				mouthBeam.GetComponent<Renderer>().material.color = Color.red;
				//ball.transform.GetComponent<Rigidbody>().AddForce(mouthForceNormalized * 
				                       //                           (150/ballMouthDis)*200);
				ball.transform.GetComponent<Rigidbody>().AddForce(mouthForceNormalized * 
				                                                  (ballMouthDis + 50));
			}

		} else {
			mouthBeam.GetComponent<Renderer> ().material.color = Color.blue;
		}

	}

	Matrix4x4 PerspectiveOffCenter(float left, float right, float bottom, float top, float near, float far) {
		float x = 2.0F * near / (right - left);
		float y = 2.0F * near / (top - bottom);
		float a = (right + left) / (right - left);
		float b = (top + bottom) / (top - bottom);
		float c = -(far + near) / (far - near);
		float d = -(2.0F * far * near) / (far - near);
		float e = -1.0F;
		Matrix4x4 m = new Matrix4x4();
		m[0, 0] = x;
		m[0, 1] = 0;
		m[0, 2] = a;
		m[0, 3] = 0;
		m[1, 0] = 0;
		m[1, 1] = y;
		m[1, 2] = b;
		m[1, 3] = 0;
		m[2, 0] = 0;
		m[2, 1] = 0;
		m[2, 2] = c;
		m[2, 3] = d;
		m[3, 0] = 0;
		m[3, 1] = 0;
		m[3, 2] = e;
		m[3, 3] = 0;
		return m;
	}

	private bool LoadFile(string fileName)
	{
		// Handle any problems that might arise when reading the text
		try
		{
			string line;
			StreamReader theReader = new StreamReader(fileName, Encoding.Default);

			using (theReader)
			{
				int count = 0;
				do
				{
					line = theReader.ReadLine();
					
					if (line != null)
					{
						float curValue = float.Parse(line);
						print (curValue);
						if (count == 3) {
							screenWidth= curValue;
						} else if (count == 4) {
							screenHeight = curValue;
						}
						count++;
					}
				}
				while (line != null);
  
				theReader.Close();
				return true;
			}
		}

		catch (Exception e)
		{
			Console.WriteLine("{0}\n", e.Message);
			return false;
		}
	}

}
