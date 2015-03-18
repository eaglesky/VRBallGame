using UnityEngine;
using System.Collections;

public class FrameManager : MonoBehaviour {
	public GameObject ball;
	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		transform.position=new Vector3(transform.position.x,transform.position.y, ball.transform.position.z); 
	}
}
