using UnityEngine;
public class CameraFollow : MonoBehaviour
{
    GameObject ball;
    // Start is called before the first frame update
    void Start()
    {
        ball = GameObject.Find("Ball");
        GameObject[] coins = GameObject.FindGameObjectsWithTag("Coin");
        for (int i = 0; i < coins.Length; i++)
        {
            Debug.Log(coins[i].name);
        }
    }
    // Update is called once per frame
    void Update()
    {
        transform.position = new Vector3(0,
        ball.transform.position.y + 3,
        ball.transform.position.z - 14);
    }
}