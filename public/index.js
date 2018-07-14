// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    
    $("#statusField").scrollTop = $("#statusField").scrollHeight;
    //create table IF NOT EXISTS INDIVIDUAL (ind_id int PRIMARY KEY AUTO_INCREMENT, surname VARCHAR(256) NOT NULL, given_name varchar(256) not null, sex varchar(1), fam_size int, source_file INT, foreign key(source_file) references FILE(file_id) on delete cascade);
	//create table IF NOT EXISTS  FILE (file_id INT PRIMARY KEY AUTO_INCREMENT, file_Name VARCHAR(60), source VARCHAR(250), version VARCHAR(10), encoding VARCHAR(10), sub_name VARCHAR(62), sub_addr VARCHAR(256), num_individials int, num_families int); 
    function readFiles(){
        
        $.ajax({
            type: 'get',
            dataType: 'json',
            url:'/uploads/',
            success: function(data){
				var currentText = $("#statusField").val();
				var newText = "";
				for(var i = 0; i < data.length; i++){
					if(data[i].hasOwnProperty('filePath')){
						if(i == 0){
							getFileData(data[0].filePath);
						}
						var newLine = data[i].filePath.toString();
						newText += newLine + " was parsed correctly\n";
												
						var table = document.getElementById("fileTable");		
						var tableLength = table.rows.length;				
						var newRow = table.insertRow(tableLength);
						
						var fileName = newRow.insertCell(0);
						fileName.innerHTML = "<a href=\"" + data[i].filePath + "\">" + data[i].fileName + "</a>";					
						
						var source = newRow.insertCell(1);
						source.innerHTML = data[i].source;
						
						var gedVer = newRow.insertCell(2);
						gedVer.innerHTML = data[i].gedVersion;
						
						var encoding = newRow.insertCell(3);
						encoding.innerHTML = data[i].encoding;
						
						var subName = newRow.insertCell(4);
						subName.innerHTML = data[i].subName;
						
						var subAddr = newRow.insertCell(5);
						subAddr.innerHTML = data[i].subAddr;
						
						var indiSize = newRow.insertCell(6);
						indiSize.innerHTML = data[i].indiSize;
						
						var famSize = newRow.insertCell(7);
						famSize.innerHTML = data[i].famSize;
						
						addToDropdown(data[i]);
						
					} else{
						var newLine = data[i].fileName.toString();
						newText += newLine + " was not properly parsed\n";
					}
				}
				currentText += newText;	
				$("#statusField").val(currentText);
				
				
				$('#fileTable a').click(function(e){
					var currentText = $("#statusField").val();
					var fileLoc = $(this).attr("href");
					var newText = currentText + "\n" + fileLoc.substr(10) + " downloaded";
					$("#statusField").val(newText);
				});
				
            },
            fail: function(error){
				var currentText = $("#statusField").val();
				var newLine = "Error while reading file directory"
				var newText = currentText + "\n" + newLine + "";
				$("#statusField").val(newText);
                console.log(error);
            }
        });
    }
    
    
    function addToDropdown(obj){
		var x = document.getElementById("fileList");
		var y = document.getElementById("fileListIndi");
		var z = document.getElementById("fileListAnces");
		var w = document.getElementById("fileListDesc");
		
		var option1 = document.createElement("option");
		option1.text = obj.fileName;
		var option2 = document.createElement("option");
		option2.text = obj.fileName;
		var option3 = document.createElement("option");
		option3.text = obj.fileName;
		var option4 = document.createElement("option");
		option4.text = obj.fileName;
		x.add(option1, x[0]);
		y.add(option2, y[0]);
		z.add(option3, z[0]);
		w.add(option4, w[0]);
		
	    $('#fileList').change( function(e) {
		var fileName = $('#fileList').val();
		var filePath = "./uploads/" + fileName;
		getFileData(filePath);
		
		});
	}
    
    getLogin();
    readFiles();
    
    function rereadFiles(){
		    $.ajax({
            type: 'get',
            dataType: 'json',
            url:'/uploads/',
            success: function(data){
				
				var toRemove = document.getElementById("fileTable");
				var tLength = toRemove.rows.length;
				
				for(var i = 0; i < tLength-1; i++){
					toRemove.deleteRow(1);
				}
					
				var currentText = $("#statusField").val();
				var newText = "";
				for(var i = 0; i < data.length; i++){
					if(data[i].hasOwnProperty('filePath')){
												
						var table = document.getElementById("fileTable");		
						var tableLength = table.rows.length;				
						var newRow = table.insertRow(tableLength);
						
						var fileName = newRow.insertCell(0);
						fileName.innerHTML = "<a href=\"" + data[i].filePath + "\">" + data[i].fileName + "</a>";					
						
						var source = newRow.insertCell(1);
						source.innerHTML = data[i].source;
						
						var gedVer = newRow.insertCell(2);
						gedVer.innerHTML = data[i].gedVersion;
						
						var encoding = newRow.insertCell(3);
						encoding.innerHTML = data[i].encoding;
						
						var subName = newRow.insertCell(4);
						subName.innerHTML = data[i].subName;
						
						var subAddr = newRow.insertCell(5);
						subAddr.innerHTML = data[i].subAddr;
						
						var indiSize = newRow.insertCell(6);
						indiSize.innerHTML = data[i].indiSize;
						
						var famSize = newRow.insertCell(7);
						famSize.innerHTML = data[i].famSize;
						
						addToDropdown(data[i]);
						
					} else{
					
					}
				}
				
				$('#fileTable a').click(function(e){
					var currentText = $("#statusField").val();
					var fileLoc = $(this).attr("href");
					var newText = currentText + "\n" + fileLoc.substr(10) + " downloaded";
					$("#statusField").val(newText);
				});
				
            },
            fail: function(error){
				var currentText = $("#statusField").val();
				var newLine = "Error while reading file directory"
				var newText = currentText + "\n" + newLine + "";
				$("#statusField").val(newText);
                console.log(error);
            }
        });
	}
    
    function getFileData(filePath){
		
		 $.ajax({
            type: 'get',
            dataType: 'json',
            data: {
				'fileName': filePath
			},
            url:'/indiList/',
            success: function(data){
				var toRemove = document.getElementById("indiTable");
				var tLength = toRemove.rows.length;
				
				for(var i = 0; i < tLength-1; i++){
					toRemove.deleteRow(1);
				}
				
				for(var i = 0; i < data.length; i++){
					var indi = data[i];
					var table = document.getElementById("indiTable");
					var tableLength = table.length;
					var newRow = table.insertRow(tableLength);
					
					var fName = newRow.insertCell(0);
					fName.innerHTML = data[i].givenName;
					
					var lName = newRow.insertCell(1);
					lName.innerHTML = data[i].surname;
				}
			},
			fail: function(error){
				
				console.log(error);
			}
		
		});
	}
	
	function getLogin(){
		$('#login').modal({
			backdrop : 'static',
			keyboard: false
		});
	}

	
    $('#fileList').change( function(e) {
		var fileName = $('#fileList').val();
		var filePath = "./uploads/" + fileName;
		getFileData(filePath);
		
	});
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/someendpoint',   //The server endpoint we are connecting to
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */

            //We write the object to the console to show that the request was successful
            console.log(data); 
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
        
    });
	
    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#someform').submit(function(e){
        e.preventDefault();
        $.ajax({});
        $.get('index.html', null, );
    });
    
    $('#submitAdd').click(function(e){
		var fileName = $("#fileListIndi").val();
		var filePath = "./uploads/" + fileName;
		var obj = {'givenName': $("#givenName").val(),
					'surname': $("#surName").val()}
		$.ajax({
			type: 'get',
			data: {
				'obj': obj,
				'fileName': "./uploads/" + fileName
			},
			dataType: 'json',
			url: '/addIndi/',
			success: function(data){
				var currentText = $("#statusField").val();
				var newText = "Individial was added succesfully\n";
				alert(newText);
				currentText += newText;	
				$("#statusField").val(currentText);
				rereadFiles();
				
				if($('#fileList').val() == fileName){
					getFileData(filePath);
				}
				
			},
			fail: function(error){
				
				console.log(error);
			}
		});
	});
    
    $('#store').click(function(e){
		deleteFiles();
		$.ajax({
            type: 'get',
            dataType: 'json',
            url:'/uploads/',
            success: function(data){
				
				for(var i = 0; i < data.length; i++){
					if(data[i].hasOwnProperty('filePath')){							
						
						var source = data[i].source;
						
						var gedVer = data[i].gedVersion;
						
						var encoding = data[i].encoding;
						
						var subName = data[i].subName;
						
						var subAddr = "'" + data[i].subAddr + "'";
						if(subAddr == "''")
							subAddr = "NULL";
							
						var indiSize = data[i].indiSize;
						
						var famSize = data[i].famSize;
						var sqlStatement = "INSERT INTO FILE (file_Name, source, version, encoding, sub_name, sub_addr, num_individials, num_families)";
						sqlStatement += " VALUES ('"+ data[i].fileName + "','" + source + "','" + gedVer + "','" + encoding + "','" + subName + "'," + subAddr + "," + indiSize + "," + famSize + ");"
						sendQuery(sqlStatement);
						saveIndi(data[i].filePath, data[i].fileName);
						
					} else{
						
					}
				}
				 getCount();
            },
            fail: function(error){
                console.log(error);
            }
        });
	});
    
    function saveIndi(filePath, fileName){
			$.ajax({
            type: 'get',
            dataType: 'json',
            data: {
				'fileName': filePath
			},
            url:'/indiList/',
            success: function(data){
				
				for(var i = 0; i < data.length; i++){
					var fName = data[i].givenName;
					var lName = data[i].surname;
					
					var sqlStatement = "INSERT INTO INDIVIDUAL (surname, given_name, sex, fam_size, source_file)";
					//"SELECT file_id from FILE where file_Name = '" + fileName + "'"
					sqlStatement += " VALUES ('" + lName + "','" + fName + "', NULL, 0, " + "(SELECT file_id from FILE where file_Name = '" + fileName + "')" +");";
					sendQuery(sqlStatement);
				}
			},  
			fail: function(error){ 
				 
				console.log(error);
			}
		 
		});
	}
	
    $('#dbStatus').click(function(e){
	
		getCount();

	});
	
	$('#qLastName').click(function(e){
		
		var queryText = 'SELECT * FROM INDIVIDUAL ORDER BY surname ASC;';
		
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){
				var toRemove = document.getElementById("qTable");
				var tLength = toRemove.rows.length;
				
				for(var i = 0; i < tLength-1; i++){
					toRemove.deleteRow(1);
				}
				
				for(var i = 0; i < data.length; i++){
					var indi = data[i];
					var table = document.getElementById("qTable");
					var tableLength = table.length;
					var newRow = table.insertRow(tableLength);
					
					var fName = newRow.insertCell(0);
					fName.innerHTML = data[i].given_name;
					
					var lName = newRow.insertCell(1);
					lName.innerHTML = data[i].surname;
				}
			}, 
			fail: function(error){
				console.log(error);
			}
		});
		
	});
	
	
	$('#q5Longer').click(function(e){
		
		var queryText = 'SELECT * FROM INDIVIDUAL WHERE LENGTH(given_name) > 5;';
		
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){
				var toRemove = document.getElementById("qTable");
				var tLength = toRemove.rows.length;
				
				for(var i = 0; i < tLength-1; i++){
					toRemove.deleteRow(1);
				}
				
				for(var i = 0; i < data.length; i++){
					var indi = data[i];
					var table = document.getElementById("qTable");
					var tableLength = table.length;
					var newRow = table.insertRow(tableLength);
					
					var fName = newRow.insertCell(0);
					fName.innerHTML = data[i].given_name;
					
					var lName = newRow.insertCell(1);
					lName.innerHTML = data[i].surname;
				}
			}, 
			fail: function(error){
				console.log(error);
			}
		});
		
	});
	
	$('#qSpecLetter').click(function(e){
		var letter = prompt("Please enter a letter", "a");
		var queryText = 'SELECT * FROM INDIVIDUAL WHERE given_name LIKE "' + letter +'%";';
		
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){
				var toRemove = document.getElementById("qTable");
				var tLength = toRemove.rows.length;
				
				for(var i = 0; i < tLength-1; i++){
					toRemove.deleteRow(1);
				}
				
				for(var i = 0; i < data.length; i++){
					var indi = data[i];
					var table = document.getElementById("qTable");
					var tableLength = table.length;
					var newRow = table.insertRow(tableLength);
					
					var fName = newRow.insertCell(0);
					fName.innerHTML = data[i].given_name;
					
					var lName = newRow.insertCell(1);
					lName.innerHTML = data[i].surname;
				}
			}, 
			fail: function(error){
				console.log(error);
			}
		});
		
	});
	
	$('#qEnds').click(function(e){
		var letter = prompt("Please enter an ending", "re");
		var queryText = 'SELECT * FROM INDIVIDUAL WHERE given_name LIKE "%' + letter +'";';
		
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){
				var toRemove = document.getElementById("qTable");
				var tLength = toRemove.rows.length;
				
				for(var i = 0; i < tLength-1; i++){
					toRemove.deleteRow(1);
				}
				
				for(var i = 0; i < data.length; i++){
					var indi = data[i];
					var table = document.getElementById("qTable");
					var tableLength = table.length;
					var newRow = table.insertRow(tableLength);
					
					var fName = newRow.insertCell(0);
					fName.innerHTML = data[i].given_name;
					
					var lName = newRow.insertCell(1);
					lName.innerHTML = data[i].surname;
				}
			}, 
			fail: function(error){
				console.log(error);
			}
		});
		
	});
	
	$('#qCustom').click(function(e){
		
		var queryText = $("#cStatement").val();
		
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){
				var toRemove = document.getElementById("qTable");
				var tLength = toRemove.rows.length;
				
				for(var i = 0; i < tLength-1; i++){
					toRemove.deleteRow(1);
				}
				
				for(var i = 0; i < data.length; i++){
					var indi = data[i];
					var table = document.getElementById("qTable");
					var tableLength = table.length;
					var newRow = table.insertRow(tableLength);
					
					var fName = newRow.insertCell(0);
					fName.innerHTML = data[i].given_name;
					
					var lName = newRow.insertCell(1);
					lName.innerHTML = data[i].surname;
				}
			}, 
			fail: function(error){
				console.log(error);
			}
		});
		
	});
	
	
	$('#qFromFile').click(function(e){
		var fileName = prompt("Please enter file name", "example.ged");
		var queryText = 'SELECT * FROM INDIVIDUAL WHERE source_file = (Select file_id from FILE where file_Name = "' + fileName + '");';
		
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){
				var toRemove = document.getElementById("qTable");
				var tLength = toRemove.rows.length;
				
				for(var i = 0; i < tLength-1; i++){
					toRemove.deleteRow(1);
				}
				
				for(var i = 0; i < data.length; i++){
					var indi = data[i];
					var table = document.getElementById("qTable");
					var tableLength = table.length;
					var newRow = table.insertRow(tableLength);
					
					var fName = newRow.insertCell(0);
					fName.innerHTML = data[i].given_name;
					
					var lName = newRow.insertCell(1);
					lName.innerHTML = data[i].surname;
				}
			}, 
			fail: function(error){
				console.log(error);
			}
		});
		
	});
	
	
	function getCount(){
		var indiCount = 0;
		var fileCount = 0;
		var queryText = "select count(file_id) as count from FILE;";
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){
				
				var currentText = $("#statusField").val();
				var newLine = "Database has " + data[0].count + " Files";
				var newText = currentText + "\n" + newLine + "";
				$("#statusField").val(newText);
			}, 
			fail: function(error){
				console.log(error);
			}
		});
		queryText = "select count(ind_id) as count from INDIVIDUAL;";
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){
				var currentText = $("#statusField").val();
				var newLine = " and " + data[0].count + " individuals";
				var newText = currentText + newLine + "";
				$("#statusField").val(newText);
			}, 
			fail: function(error){
				console.log(error);
			}
		});
	}
	
    function sendQuery(queryText){
		$.ajax({
			type: 'get',
			data: {
				'query' : queryText
			},
			dataType: 'json',
			url: '/query/',
			success: function(data){

			}, 
			fail: function(error){
				console.log(error);
			}
		});
	}
    
	$('#submitDB').click(function(e){
		var user = $("#user").val();
		var pass = $("#pass").val();
		var dbname = $("#dbname").val();
		$.ajax({
			type: 'get',
			data: {
				'userName' : user,
				'password' : pass,
				'database' : dbname
			},
			dataType: 'json',
			url: '/connect/',
			success: function(data){
				if(data.type != 'ok'){
					alert("Error in login. Please try again");
				}else{
					alert("SUCCESS!");
					$('#login').modal('hide');
				}
			},
			fail: function(error){
				console.log(error);
			}
		});
	});
    
    function deleteFiles(){
		var sqlStatement = "Delete from FILE;";
		sendQuery(sqlStatement);
	}
    
    $('#clear').click(function(e){
		deleteFiles();
		getCount();
	});
    
    $('#submitCreate').click(function(e){
        var fileName = $("#fileName").val();
        if(fileName === ""){
            alert("File name required");
            return;
        }
        var submitterName = $("#subName").val();
        if(submitterName === ""){
            alert("Submitter name required");
            return;
        }
        var obj = {"source" : "2750Parser",
				   "gedcVersion": "5.5",
				   "encoding": "UTF8",
				   "subName": $("#subName").val(),
				   "subAddress": $("#subAddr").val() }
				   
		$.ajax({
			type: 'get', 
			dataType: 'json',
			data: {
				'obj': obj,
				'fileName': "./uploads/" + fileName + ".ged"
			},
			url: '/simpleGED/',
			success: function (data) {
				var currentText = $("#statusField").val();
				var newText = fileName + " was created correctly\n";
				alert(newText);
				currentText += newText;	
				$("#statusField").val(currentText);
				
				
				var table = document.getElementById("fileTable");		
				var tableLength = table.rows.length;
				var newRow = table.insertRow(tableLength);
					
				
				var fileLink = newRow.insertCell(0);
				fileLink.innerHTML = "<a href= \"./uploads/" + fileName + ".ged\">" + fileName +"</a>";
									
				
				var source = newRow.insertCell(1);
				source.innerHTML = data.source;
				
				var gedVer = newRow.insertCell(2);
				gedVer.innerHTML = data.gedcVersion;
				
				var encoding = newRow.insertCell(3);
				encoding.innerHTML = data.encoding;
				
				var subName = newRow.insertCell(4);
				subName.innerHTML = data.subName;
				
				var subAddr = newRow.insertCell(5);
				subAddr.innerHTML = data.subAddress;
				
				var indiSize = newRow.insertCell(6);
				indiSize.innerHTML = "0";
				
				var famSize = newRow.insertCell(7);
				famSize.innerHTML = "0";
				
				data.fileName = fileName +".ged";
				addToDropdown(data);
			},
			fail: function(error) {
				console.log(error); 
			}
			
		});
        
    });
    
});
