using UnityEngine;
using System.Collections;

public class BallManager : MonoBehaviour {
	public GameObject board;
	public GameObject indicator;
	public int score = 0;

	// Use this for initialization
	void Start () {
	}
	
	// Update is called once per frame
	void Update () {
	}

	void OnCollisionEnter(Collision mCollision)  
	{  
		if(mCollision.gameObject.tag=="Target")  
		{  
			Destroy(mCollision.gameObject); 
			score += 100;
		}
		else if(mCollision.gameObject.tag=="TargetPole")  
		{  
			Destroy(mCollision.gameObject); 
			score += 30;
		}  
	}  
}
