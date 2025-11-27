using UnityEngine;
using UnityEngine.SceneManagement;
public class GameMagager : MonoBehaviour
{
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
    void RestartGame()
    {
        SceneManager.LoadScene(0);
    }
    void RedCoinStart()
    {
        DestroyObstacles();
    }
    void DestroyObstacles()
    {
        GameObject[] obstacles = GameObject.FindGameObjectsWithTag("Obstacle");
        for (int i = 0; i < obstacles.Length; i++)
        {
            Destroy(obstacles[i]);
        }
    }
    int coinCount = 0;
    void GetCoin()
    {
        coinCount++;
        Debug.Log("µ¿Àü : " + coinCount);
    }


}

