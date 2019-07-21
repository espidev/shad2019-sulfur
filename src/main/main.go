package main

import (
	"context"
	"github.com/gin-gonic/gin"
	"log"
	"net/http"
	"os"
	"os/signal"
	"time"
)

var (
	router *gin.Engine
	flowVol int64
	totalVol int64
)

const (
	RootFolder = "."
)

type Req struct {
	FlowVolume int64 `json:"flow_volume"`
	TotalVolume int64 `json:"total_volume"`
}

func main() {
	log.Println("Starting SHAD2019 Sulfur Demo...")

	router = gin.Default()

	// setup routes

	router.Use(gin.Logger())
	router.Use(gin.Recovery())

	router.LoadHTMLGlob(RootFolder + "/src/assets/html/*")
	router.Static("/images", RootFolder+ "/assets/images")
	router.GET("/", func (c *gin.Context) {
		c.HTML(http.StatusOK, "index.html", gin.H{
			"flowTotal": totalVol,
			"flowRate": flowVol,
		})
	})
	router.GET("/flowvol", func(c *gin.Context) {
		c.JSON(200, gin.H{"amount": flowVol})
	})
	router.GET("/totalvol", func(c *gin.Context) {
		c.JSON(200, gin.H{"amount": totalVol})
	})

	router.POST("/update", func (c *gin.Context) {
		var req Req
		err := c.BindJSON(&req)
		if err != nil {
			log.Println(err)
			c.JSON(400, gin.H{})
			return
		}
		flowVol = req.FlowVolume
		totalVol = req.TotalVolume
		c.JSON(200, gin.H{})
	})

	// do server start
	srv := &http.Server {
		Addr: ":3001",
		Handler: router,
	}

	go func () {
		if err := srv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("listen %s\n", err)
		}
	}()

	// listen for sigint to shutdown gracefully
	quit := make(chan os.Signal)
	signal.Notify(quit, os.Interrupt)
	<-quit
	log.Println("Shutting down SHAD2019-Sulfur...")

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	if err := srv.Shutdown(ctx); err != nil {
		log.Fatal("Server shutdown: ", err)
	}
	log.Println("SHAD2019-Sulfur has stopped.")
}
