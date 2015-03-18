using UnityEngine;
using System.Collections;

public class MissileManager : MonoBehaviour {

	public float zSpeed;
	private bool activated = true;
	
	public GameObject breakGlassSprite;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		transform.RotateAround (transform.position, new Vector3 (0F, 0F, 1F), 5F);
		transform.position = new Vector3 (transform.position.x, transform.position.y, transform.position.z - zSpeed);

		Camera camera = Camera.current;
		GameObject ball = GameObject.FindGameObjectWithTag ("Ball");
		if (activated && transform.position.z < -CameraController.boxLength / 2F && CameraController.ballInPlay) {
			if (System.Math.Abs(transform.position.x - camera.transform.position.x) < 3 &&
			    System.Math.Abs(transform.position.y - camera.transform.position.y) < 3) {
				activated = false;
				CameraController.ballInPlay = false;
				ball.SetActive(false);

				Instantiate (breakGlassSprite, transform.position, Quaternion.Euler (new Vector3 (0.0F, 0.0F, 0.0F)));
			}
			else {
				activated = false;
			}
		}
		if (transform.position.z < -CameraController.boxLength)
			Destroy (gameObject);
	}
}
