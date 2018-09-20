package golang

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"net"
	"reflect"
	"time"

	"github.com/vmihailenco/msgpack"
)

const (
	REQUEST      = 0
	RESPONSE     = 1
	NOTIFICATION = 2
)

var ReconnectInterval time.Duration = time.Second * 1

type Client struct {
	conn          net.Conn
	nextId        uint32
	connTimeout   time.Duration
	network       string
	addr          string
	autoReconnect bool
	disconnected  bool
	connTime      time.Time
}

func NewClient(conn net.Conn) *Client {
	addr := conn.RemoteAddr()
	return &Client{conn, 1, time.Duration(0), addr.Network(), addr.String(), false, false, time.Now()}
}

func Connect(network, addr string, timeout ...time.Duration) (*Client, error) {
	var timeout_ time.Duration
	if len(timeout) > 0 {
		timeout_ = timeout[0]
	}
	c, err := connect(network, addr, timeout_)
	if err != nil {
		return nil, err
	}
	return &Client{c, 1, timeout_, network, addr, false, false, time.Now()}, nil
}

func (this *Client) Reconnect() (err error) {
	this.connTime = time.Now()
	if this.conn != nil {
		this.conn.Close()
	}
	this.conn, err = connect(this.network, this.addr, this.connTimeout)
	if err == nil {
		this.disconnected = false
	}
	return
}

func connect(network, addr string, timeout time.Duration) (conn net.Conn, err error) {
	if timeout <= 0 {
		conn, err = net.Dial(network, addr)
	} else {
		conn, err = net.DialTimeout(network, addr, timeout)
	}
	return
}

func (this *Client) Close() {
	this.conn.Close()
}

func (this *Client) SetAutoReconnect(autoReconnect bool) {
	this.autoReconnect = autoReconnect
}

func (this *Client) SetDeadline(timeout time.Duration) {
	this.conn.SetDeadline(time.Now().Add(timeout))
}

func (this *Client) SetReadDeadline(timeout time.Duration) {
	this.conn.SetReadDeadline(time.Now().Add(timeout))
}

func (this *Client) SetWriteDeadline(timeout time.Duration) {
	this.conn.SetWriteDeadline(time.Now().Add(timeout))
}

func (this *Client) checkConnect() (err error) {
	if !this.disconnected || !this.autoReconnect {
		return
	}
	if time.Now().Sub(this.connTime) > ReconnectInterval {
		err = this.Reconnect()
	} else {
		err = errors.New("Connection is disconnected.")
	}
	return
}

func (this *Client) handleError(err error) {
	if !this.autoReconnect {
		return
	}
	if err != nil {
		this.disconnected = true
	}
}

func (this *Client) Call(funcName string, arguments ...interface{}) (result reflect.Value, err error) {
	err = this.checkConnect()
	if err != nil {
		return
	}

	var msgId = this.nextId
	this.nextId += 1
	err = this.request(this.conn.(io.Writer), msgId, funcName, arguments...)
	if err != nil {
		return
	}
	var _msgId uint32
	_msgId, result, err = this.response(this.conn.(io.Reader))
	if err != nil {
		return
	}
	if msgId != _msgId {
		err = errors.New(fmt.Sprintf("Message IDs don't match (%d != %d)", msgId, _msgId))
	}
	return
}

func (this *Client) request(writer io.Writer, msgId uint32, funcName string, arguments ...interface{}) (err error) {
	var buf bytes.Buffer
	buf.WriteByte(0x94)
	enc := msgpack.NewEncoder(&buf)
	err = enc.EncodeMulti(uint8(REQUEST), msgId, funcName, arguments)
	if err != nil {
		this.handleError(err)
		return
	}
	_, err = writer.Write(buf.Bytes())
	if err != nil {
		this.handleError(err)
	}
	return
}

func (this *Client) response(reader io.Reader) (uint32, reflect.Value, error) {
	var err error
	var data interface{}
	var msgType int
	var msgId int
	dec := msgpack.NewDecoder(reader)
	data, err = dec.DecodeInterface()
	if err != nil {
		this.handleError(err)
		return 0, reflect.Value{}, err
	}
	for {
		_data, ok := data.([]interface{})
		if !ok {
			break
		}
		if len(_data) != 4 {
			break
		}
		msgType, err = this.toInteger(_data[0])
		if err != nil {
			break
		}
		if msgType != RESPONSE {
			break
		}
		msgId, err = this.toInteger(_data[1])
		errMsg := reflect.ValueOf(_data[2])
		if errMsg.IsValid() {
			err = errors.New(fmt.Sprint(errMsg))
			break
		}
		return uint32(msgId), reflect.ValueOf(_data[3]), nil
	}
	if err == nil {
		err = errors.New("Invalid message format")
	}
	return uint32(msgId), reflect.Value{}, err
}

func (this *Client) Send(funcName string, arguments ...interface{}) (err error) {
	err = this.checkConnect()
	if err != nil {
		return
	}

	var buf bytes.Buffer
	buf.WriteByte(0x93)
	enc := msgpack.NewEncoder(&buf)
	err = enc.EncodeMulti(uint8(NOTIFICATION), funcName, arguments)
	if err != nil {
		this.handleError(err)
		return
	}
	_, err = this.conn.Write(buf.Bytes())
	if err != nil {
		this.handleError(err)
	}
	return
}

func (this *Client) toInteger(v interface{}) (int, error) {
	switch v_ := v.(type) {
	case int8:
		return int(v_), nil
	case int16:
		return int(v_), nil
	case int32:
		return int(v_), nil
	case int64:
		return int(v_), nil
	case uint8:
		return int(v_), nil
	case uint16:
		return int(v_), nil
	case uint32:
		return int(v_), nil
	case uint64:
		return int(v_), nil
	case int:
		return v_, nil
	}
	return 0, errors.New("Invalid message format")
}
