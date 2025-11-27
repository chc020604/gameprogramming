using UnityEditor.SearchService;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class GameResult : MonoBehaviour
{
    public int highScore;
    public Text resultTime;
    public Text BestTime;
    public GameObject parts;
    void Start()
    {
        if (PlayerPrefs.HasKey("HighScore"))
        {
            highScore = PlayerPrefs.GetInt("HighScore");
        }
        else
        {
            highScore = 999;
        }
    }

    // Update is called once per frame
    void Update()
    {
        if(GoalArea.goal)
        {
            parts.SetActive(true);
            int result = Mathf.FloorToInt(Timer.time);
            resultTime.text = "Result Time: " + result;
            BestTime.text = "Best Time: " + highScore;

            if(result < highScore)
            {
                PlayerPrefs.SetInt("HighScore", result);
            }
        }
    }
    public void OnRetry()
    {
        SceneManager.LoadScene("Main");
    }
}
