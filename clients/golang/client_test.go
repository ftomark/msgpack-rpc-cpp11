package golang

import (
	"fmt"
	"testing"
	"time"
)

func Test_Send(t *testing.T) {
	client, err := Connect("tcp", "192.168.254.129:8080")
	if err != nil {
		panic(err)
	}
	defer client.Close()
	//client.SetAutoReconnect(true)

	s := time.Now()
	err = client.Send("foo", "hello world!", 3, float32(345.23), []int32{1, 2, 3, 4}, map[string]string{"aaa": "111", "bbb": "222"})
	if err != nil {
		fmt.Printf("foo() => %v, cost time:%v\n", err, time.Now().Sub(s))
	} else {
		fmt.Printf("foo() => ok, cost time:%v\n", time.Now().Sub(s))
	}
	s = time.Now()
	rs, err := client.Call("add", 3, 8)
	if err != nil {
		fmt.Printf("add(3,5) => %v, cost time:%v\n", err, time.Now().Sub(s))
	} else {
		fmt.Printf("add(3,5) => %v, cost time:%v\n", rs, time.Now().Sub(s))
	}

}
